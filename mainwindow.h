#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "difftext.h"

#include <QMainWindow>
#include <QString>
#include <QTextBlockFormat>

namespace Ui {
class MainWindow;
}

class QPlainTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_diffButton_clicked();
    void on_editButton_clicked();

private:
    void enterDiff();
    void enterEdit();
    void doDiff();
    void showDiff(const DiffText &text);
    void initWidget();
    void connectScroll();
    void disconnectScroll();
    void initTextFormat();

    void setDiffLines2Edit(const std::vector<std::shared_ptr<DiffLine>> &lines, QPlainTextEdit *edit);

private:
    Ui::MainWindow *ui;

    const static QChar TEXTEDIT_DELIMITER;
    const static QChar STRING_DELIMITER;

    QString m_leftPlainText;
    QString m_rightPlainText;

    QString m_leftRawText;
    QString m_rightRawText;

    QTextBlockFormat m_equalFormat;
    QTextBlockFormat m_addFormat;
    QTextBlockFormat m_removeFormat;
    QTextBlockFormat m_replaceFormat;
    QTextBlockFormat m_emptyFormat;

    QMetaObject::Connection m_vLeft2Right;
    QMetaObject::Connection m_vRight2Left;
    QMetaObject::Connection m_hLeft2Right;
    QMetaObject::Connection m_hRight2Left;
};

#endif // MAINWINDOW_H
