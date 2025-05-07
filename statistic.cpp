#include "statistic.h"
#include <QVBoxLayout>

Statistic::Statistic(QWidget *parent) : QWidget(parent)
{
    // Создаем виджет графика
    QVBoxLayout *layout = new QVBoxLayout(this);

    QChartView *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing); // сглаживание
    layout->addWidget(chartView);
    setLayout(layout);

    // Вызываем метод построения графика
    createTestChart();
}

void Statistic::createClientsChart()
{

}



void Statistic::createTestChart()
{
    // Создаем график
    QChart *chart = new QChart();
    chart->animationOptions() |= QChart::AllAnimations;
    chart->setTitle("Тестовый график роста записей");

    // Создаем набор данных
    QBarSeries *series = new QBarSeries();

    QBarSet *set0 = new QBarSet("Количество");
    *set0 << 3 << 5 << 7 << 4 << 10;

    series->append(set0);

    chart->addSeries(series);

    // Настройка осей
    QStringList categories;
    categories << "Пн" << "Вт" << "Ср" << "Чт" << "Пт";

    // Создаем категориальную ось X
    QCategoryAxis *axisX = new QCategoryAxis();
    for (int i = 0; i < categories.size(); ++i) {
        axisX->append(categories[i], i); // Первый аргумент — текст, второй — значение
    }

    // Создаем числовую ось Y
    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d"); // Отображать целые числа

    // Добавляем оси к графику
    chart->addAxis(axisX, Qt::AlignBottom); // Ось X внизу
    chart->addAxis(axisY, Qt::AlignLeft);  // Ось Y слева

    // Связываем серию с осями
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // Легенда
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Назначаем график в QChartView
    QChartView *chartView = qobject_cast<QChartView*>(layout()->itemAt(0)->widget());
    if (chartView) {
        chartView->setChart(chart);
    }
}
