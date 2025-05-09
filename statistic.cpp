#include "statistic.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSqlQuery>
#include <QMessageBox>
#include <QPainter>
#include <QDate>
#include <QToolTip>
#include <QLineSeries>

Statistic::Statistic(QWidget *parent, Database *db) : QWidget(parent), database(db), chart(new QChart()) {
    createUI();
    chartView->setChart(chart); // Устанавливаем сразу
    updateChart(); // Загрузка данных по умолчанию (например, три месяца)
}

void Statistic::createUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Контрол выбора периода
    QFormLayout *controlsLayout = new QFormLayout;

    periodSelector = new QComboBox();
    periodSelector->addItem("1 месяц", ONE_MONTH);
    periodSelector->addItem("2 месяца", TWO_MONTH);
    periodSelector->addItem("3 месяца", THREE_MONTH);
    periodSelector->addItem("6 месяцев", SIX_MONTH);
    periodSelector->addItem("1 год", ONE_YEAR);
    periodSelector->addItem("2 года", TWO_YEAR);
    periodSelector->addItem("5 лет", FIVE_YEAR);
    connect(periodSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Statistic::updateChart);
    controlsLayout->addRow("Период:", periodSelector);

    dataTypeSelector = new QComboBox();
    dataTypeSelector->addItem("Новые клиенты", Count);
    dataTypeSelector->addItem("Количество клиентов", Growth);
    connect(dataTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Statistic::updateChart);
    controlsLayout->addRow("Статистика:", dataTypeSelector);

    // Контрол выбора типа графика
    chartTypeSelector = new QComboBox();
    chartTypeSelector->addItem("Столбчатый", BarChart);
    chartTypeSelector->addItem("Линейный", LineChart);
    connect(chartTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Statistic::updateChart);
    controlsLayout->addRow("Тип графика:", chartTypeSelector);

    mainLayout->addLayout(controlsLayout);



    // График
    chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(chartView);

    setLayout(mainLayout);
}

void Statistic::updateChart() {
    PeriodeMode mode = static_cast<PeriodeMode>(periodSelector->currentData().toInt());
    ChartType chartType = static_cast<ChartType>(chartTypeSelector->currentData().toInt());
    ChartDataType dataType = static_cast<ChartDataType>(dataTypeSelector->currentData().toInt());
    loadChartData(mode, chartType, dataType);
}


QDate Statistic::getStartDateFromMode(PeriodeMode mode) {
    QDate today = QDate::currentDate();
    switch (mode) {
    case ONE_MONTH:
        return today.addMonths(-1);
    case TWO_MONTH:
        return today.addMonths(-2);
    case THREE_MONTH:
        return today.addMonths(-3);
    case SIX_MONTH:
        return today.addMonths(-6);
    case ONE_YEAR:
        return today.addYears(-1);
    case TWO_YEAR:
        return today.addYears(-2);
    case FIVE_YEAR:
        return today.addYears(-5);
    default:
        return today.addMonths(-3); // По умолчанию 3 месяца
    }
}

bool Statistic::isWeeklyInterval(PeriodeMode mode) {
    switch (mode) {
    case SIX_MONTH:
    case ONE_YEAR:
    case TWO_YEAR:
    case FIVE_YEAR:
        return true; // Разбиение по неделям
    default:
        return false; // Разбиение по дням
    }
}

// Функция генерации всех дат в выбранном периоде
QStringList Statistic::generateFullDateRange(const QDate &start, const QDate &end, bool weekly) {
    QStringList dates;

    if (!weekly) {
        for (QDate date = start; date <= end; date = date.addDays(1)) {
            dates << date.toString("yyyy-MM-dd");
        }
    } else {
        QDate currentDate = start;
        while (currentDate <= end) {
            int weekNum = currentDate.weekNumber();
            QString weekKey = QString("%1-%2").arg(currentDate.year()).arg(weekNum, 2, 10, QChar('0'));

            if (!dates.contains(weekKey)) {
                dates << weekKey;
            }

            currentDate = currentDate.addDays(7);
        }
    }

    return dates;
}

void Statistic::loadChartData(PeriodeMode mode, ChartType chartType, ChartDataType dataType) {
    QDate start = getStartDateFromMode(mode);
    QDate end = QDate::currentDate().addDays(1); // Чтобы включить сегодняшнюю дату

    // Очистка серии
    for (QAbstractSeries *series : chart->series()) {
        chart->removeSeries(series);
        delete series;
    }

    // Пересоздание осей
    if (!chart->axisX()) {
        axisX = new QCategoryAxis();
        chart->addAxis(axisX, Qt::AlignBottom);
    } else {
        delete axisX;
        axisX = new QCategoryAxis();
        chart->addAxis(axisX, Qt::AlignBottom);
    }

    if (!chart->axisY()) {
        axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
    } else {
        delete axisY;
        axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
    }

    chart->setAnimationOptions(QChart::AllAnimations);

    // Определяем тип разбиения
    bool weekly = isWeeklyInterval(mode);

    // Генерируем все даты/недели в выбранном периоде
    QStringList fullDates = generateFullDateRange(start, end, weekly);

    // Подготавливаем массив значений
    QList<int> values(fullDates.size(), 0);

    QSqlQuery query(database->getDb());

    QString groupByClause = weekly ? "strftime('%Y-%W', created_at)" : "DATE(created_at)";
    query.prepare(QString("SELECT %1 AS period_key, COUNT(*) AS count "
                          "FROM clients "
                          "WHERE created_at BETWEEN :start AND :end "
                          "GROUP BY %1")
                      .arg(groupByClause));
    query.bindValue(":start", start.toString("yyyy-MM-dd"));
    query.bindValue(":end", end.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось получить данные: " + query.lastError().text());
        return;
    }


    while (query.next()) {
        QString key = query.value("period_key").toString();
        int count = query.value("count").toInt();

        int index = -1;

        if (!weekly) {
            index = fullDates.indexOf(key);
        } else {
            // Для недель сравниваем ключ в формате "2024-W15"
            //QDate date = QDate::fromString(key, "yyyy-MM-dd");
            //QString weekKey = QString("%1-%2").arg(date.year()).arg(date.weekNumber(), 2, 10, QChar('0'));
            QString weekKey = key;

            index = fullDates.indexOf(weekKey);
        }

        if (index != -1) {
            values[index] = count;
        }
    }

    int initialCount = 0;
    QSqlQuery initQuery(database->getDb());
    initQuery.prepare("SELECT COUNT(*) FROM clients WHERE created_at < :startDate");
    initQuery.bindValue(":startDate", start.toString("yyyy-MM-dd"));
    if (initQuery.exec() && initQuery.next()) {
        initialCount = initQuery.value(0).toInt();
    }

    // Считаем накопленный итог
    QList<int> cumulativeValues = values;
    int cumulative = initialCount; // ⬅️ Начинаем с уже существующих клиентов
    for (int i = 0; i < cumulativeValues.size(); ++i) {
        cumulative += values[i];
        cumulativeValues[i] = cumulative;
    }

    // Формируем финальные данные
    QList<int> finalValues = (dataType == Count) ? values : cumulativeValues;

    // Создаём серию на основе типа графика
    if (chartType == BarChart) {
        QBarSet *barSet = new QBarSet(dataType == Count ? "Клиентов" : "Накоплено");
        for (int value : finalValues) {
            barSet->append(value);
        }

        QBarSeries *series = new QBarSeries();
        series->append(barSet);
        chart->addSeries(series);

        series->attachAxis(axisX);
        series->attachAxis(axisY);

        connect(barSet, &QBarSet::hovered, this, [this, barSet, fullDates, finalValues, dataType](bool status, int index) {
            if (status && index >= 0 && index < fullDates.size()) {
                QString tooltipText = QString("Период: %1\n%2: %3")
                                          .arg(fullDates[index])
                                          .arg(dataType == Count ? "Клиентов" : "Всего клиентов")
                                          .arg(finalValues[index]);
                QToolTip::showText(QCursor::pos(), tooltipText);
            } else {
                QToolTip::hideText();
            }
        });

    } else if (chartType == LineChart) {
        QLineSeries *lineSeries = new QLineSeries();
        lineSeries->setName(dataType == Count ? "Клиентов" : "Накоплено");

        for (int i = 0; i < fullDates.size(); ++i) {
            lineSeries->append(i, finalValues[i]);
        }

        chart->addSeries(lineSeries);
        lineSeries->attachAxis(axisX);
        lineSeries->attachAxis(axisY);

        connect(lineSeries, &QLineSeries::hovered, this, [this, fullDates, finalValues, dataType](const QPointF &point, bool state) {
            int index = qRound(point.x());
            if (state && index >= 0 && index < fullDates.size()) {
                QString tooltipText = QString("Период: %1\n%2: %3")
                                          .arg(fullDates[index])
                                          .arg(dataType == Count ? "Клиентов" : "Всего клиентов")
                                          .arg(finalValues[index]);
                QToolTip::showText(QCursor::pos(), tooltipText);
            } else {
                QToolTip::hideText();
            }
        });
    }

    // Настраиваем ось X (убираем подписи)
    for (int i = 0; i < fullDates.size(); ++i) {
        axisX->append(fullDates[i], i);
    }

    axisX->setVisible(false); // ❌ Скрываем подписи на оси X

    // Настраиваем ось Y
    int maxCount = std::max_element(finalValues.begin(), finalValues.end())[0];
    axisY->setRange(0, maxCount > 0 ? maxCount + 1 : 1);
    axisY->setLabelFormat("%d");

    // Легенда
    chart->legend()->setVisible(true);

    // Заголовок графика
    chart->setTitle(
        QString("%1 (%2)\n(%3 — %4)")
            .arg(dataType == Count ? "Количество клиентов" : "Рост клиентов")
            .arg(periodSelector->currentText())
            .arg(start.toString("dd.MM.yyyy"))
            .arg(end.addDays(-1).toString("dd.MM.yyyy"))
        );
}
