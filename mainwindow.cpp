#include "mainwindow.hpp"

#include <QPainter>
#include <QPen>

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cassert>


static std::istream& operator>>(std::istream& is, QPointF& p)
{
    double x = 0.0;
    double y = 0.0;

    is >> x;
    is >> y;

    p.setX(x);
    p.setY(y);
    return is;
}

static std::vector<QPointF> LoadFisherIris(std::istream& is)
{
    std::vector<QPointF> result;
    result.reserve(150);
    std::copy(std::istream_iterator<QPointF>(is), std::istream_iterator<QPointF>{}, std::back_inserter(result));
    return result;
}

static double sqr(double x) {
    return x * x;
}

static double EuclideanDistance(const QPointF& a, const QPointF& b)
{
    return std::sqrt(sqr(a.x() - b.x()) + sqr(a.y() - b.y()));
}

static Connection PointsPairWithSmallestEuclideanDistance(const std::vector<QPointF>& points)
{
    Connection result;
    double minDistance = std::numeric_limits<double>::infinity();
    for(std::size_t i = 0; i < points.size(); ++i)
    {
        for(std::size_t j = 0; j < points.size(); ++j)
        {
            if(i == j)
                continue;

            const double dist = EuclideanDistance(points.at(i), points.at(j));
            if(dist < minDistance)
            {
                result = {i, j};
                minDistance = dist;
            }
        }
    }

    return result;
}


static std::vector<std::size_t> GenerateIsolated(std::size_t count)
{
    std::vector<std::size_t> connected(count);
    std::iota(connected.begin(), connected.end(), 0);
    return connected;
}


static void RemoveFromIsolated(std::vector<std::size_t>& isolated, std::size_t point)
{
    auto it = std::lower_bound(isolated.begin(), isolated.end(), point);
    if(it == isolated.end() || *it != point)
        return;

    isolated.erase(it);
}

static std::size_t FindClosestPoint(const std::vector<QPointF>& points,
                             const std::vector<std::size_t>& pointIndexes,
                             std::size_t firstPoint)
{
    std::size_t closest = 0;
    double minDistance = std::numeric_limits<double>::infinity();
    for(auto secondPoint : pointIndexes)
    {
        if(secondPoint == firstPoint)
            continue;

        const double dist = EuclideanDistance(points.at(firstPoint), points.at(secondPoint));
        if(dist < minDistance)
        {
            minDistance = dist;
            closest = secondPoint;
        }
    }

    return closest;
}


static std::vector<Connection> Edges(const std::vector<QPointF>& points, int k)
{
    assert(k > 0);

    std::vector<std::size_t> isolated = GenerateIsolated(points.size());
    std::vector<Connection> edges;

    const auto smallestPair = PointsPairWithSmallestEuclideanDistance(points);
    edges.emplace_back(smallestPair);
    RemoveFromIsolated(isolated, smallestPair.first);
    RemoveFromIsolated(isolated, smallestPair.second);
    while (!isolated.empty())
    {
        const auto notIsolatedPoint = edges.back().second;
        const auto closest = FindClosestPoint(points, isolated, notIsolatedPoint);
        RemoveFromIsolated(isolated, closest);
        edges.emplace_back(notIsolatedPoint, closest);
    }

    std::nth_element(edges.begin(), edges.begin() + k - 1, edges.end(), [&](auto leftEdge, auto rightEdge){
        const auto leftEdgeLength = EuclideanDistance(points.at(leftEdge.first), points.at(leftEdge.second));
        const auto rightEdgeLength = EuclideanDistance(points.at(rightEdge.first), points.at(rightEdge.second));

        return leftEdgeLength > rightEdgeLength;
    });

    edges.erase(edges.begin(), edges.begin() + k - 1);
    return edges;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(600, 400);

    std::fstream file;
    file.open("data.txt", std::ios_base::in);
    if(!file.is_open())
    {
        std::cerr << "Cannot find file data.txt!\n";
        return;
    }

    fisherIris_ = LoadFisherIris(file);
    edges_ = Edges(fisherIris_, 2);
}


void MainWindow::paintEvent(QPaintEvent*)
{
    const double scale = 150.0;
    const double shiftX = -600.0;
    const double shiftY = -290.0;

    auto scaleAndShift = [&](QPointF p) {
        p = p * scale;
        p.setX(p.x() + shiftX);
        p.setY(p.y() + shiftY);
        return p;
    };

    QPainter painter(this);
    QColor lineColor(255, 0, 0, 255);
    QColor pointColor(0, 0, 255, 255);

    QPen ppen = QPen(pointColor);
    ppen.setWidth(6);
    painter.setPen(ppen);

    for(auto p : fisherIris_)
        painter.drawPoint(scaleAndShift(p));

    QPen lpen = QPen(lineColor);
    lpen.setWidth(2);
    painter.setPen(lpen);

    for(auto edge : edges_)
        painter.drawLine(scaleAndShift(fisherIris_.at(edge.first)), scaleAndShift(fisherIris_.at(edge.second)));
}
