#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>


#include <vector>


using Connection = std::pair<std::size_t, std::size_t>;


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *) override;

private:
    std::vector<QPointF> fisherIris_;
    std::vector<Connection> edges_;
};

#endif // MAINWINDOW_HPP
