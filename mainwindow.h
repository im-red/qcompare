#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "editdistance.h"

#include <QMainWindow>
#include <QString>
#include <QTextBlockFormat>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum Side
    {
        LeftSide,
        RightSide
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_diffButton_clicked();
    void on_editButton_clicked();

private:
    void enterDiff();
    void enterEdit();
    static QString convertFromRawToPlain(const QString &raw);
    void doDiff();
    void showDiff(const std::vector<EditOperation> &ops, const QStringList &fromList, const QStringList &toList);
    void initWidget();
    void initConnection();
    void initTextFormat();

private:
    Ui::MainWindow *ui;

    const static QChar TEXTEDIT_DELIMITER;
    const static QChar ACTUAL_DELIMITER;
    const static QChar MANUAL_DELIMITER;
    const static QChar ESCAPE_CHAR;

    QString m_leftPlainText;
    QString m_rightPlainText;

    QString m_leftRawText;
    QString m_rightRawText;

    bool m_isShowing;

    QTextBlockFormat m_nopFormat;
    QTextBlockFormat m_addFormat;
    QTextBlockFormat m_removeFormat;
    QTextBlockFormat m_replaceFormat;
    QTextBlockFormat m_emptyFormat;
};

#endif // MAINWINDOW_H
