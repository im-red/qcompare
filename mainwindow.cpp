#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myersdiff.h"
#include <QDebug>
#include <QAbstractSlider>
#include <QScrollBar>
#include <QDate>
#include <QTextCodec>
#include <QFile>
#include <utility>
#include <assert.h>

const QChar MainWindow::TEXTEDIT_DELIMITER = QChar(0x2029);
const QChar MainWindow::STRING_DELIMITER = '\n';

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    showMaximized();

    initWidget();
    initTextFormat();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_diffButton_clicked()
{
    ui->editButton->setChecked(false);
    ui->editButton->setEnabled(true);
    ui->diffButton->setEnabled(false);

    ui->leftEdit->setReadOnly(true);
    ui->rightEdit->setReadOnly(true);

    ui->leftEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connectScroll();

    QTime timer;
    timer.start();
    enterDiff();
    int totalTime = timer.elapsed();
    statusBar()->showMessage(QString("%1 total time: %2s").arg(statusBar()->currentMessage()).arg(totalTime * 0.001));
}

void MainWindow::on_editButton_clicked()
{
    ui->diffButton->setChecked(false);
    ui->diffButton->setEnabled(true);
    ui->editButton->setEnabled(false);

    ui->leftEdit->setReadOnly(false);
    ui->rightEdit->setReadOnly(false);

    ui->leftEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    disconnectScroll();

    enterEdit();
}

void MainWindow::enterDiff()
{
    QTime timer;
    timer.start();
    m_leftPlainText = ui->leftEdit->toPlainText();
    m_rightPlainText = ui->rightEdit->toPlainText();
    int readTime = timer.elapsed();
    statusBar()->showMessage(QString("read time: %1s").arg(readTime * 0.001));

    doDiff();
}

void MainWindow::enterEdit()
{
    ui->leftEdit->setPlainText(m_leftPlainText);
    ui->rightEdit->setPlainText(m_rightPlainText);
}

void MainWindow::doDiff()
{
    QStringList fromList = m_leftPlainText.split(STRING_DELIMITER);
    QStringList toList = m_rightPlainText.split(STRING_DELIMITER);

    QTime timer;

    timer.start();
    auto diffText = myersDiff(fromList, toList);
    int diffTime = timer.elapsed();

    timer.restart();
    showDiff(diffText);
    int showTime = timer.elapsed();

    statusBar()->showMessage(QString("%1 diff time: %2s  show time: %3s").arg(statusBar()->currentMessage()).arg(diffTime * 0.001).arg(showTime * 0.001));
}

void MainWindow::showDiff(const DiffText &text)
{
    setDiffLines2Edit(text.fromText, ui->leftEdit);
    setDiffLines2Edit(text.toText, ui->rightEdit);
}

void MainWindow::initWidget()
{
    ui->leftEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    ui->rightEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    ui->diffButton->setCheckable(true);
    ui->diffButton->setChecked(false);
    ui->editButton->setCheckable(true);
    ui->editButton->setChecked(true);
    ui->editButton->setEnabled(false);
}

void MainWindow::connectScroll()
{
    m_vLeft2Right = connect(ui->leftEdit->verticalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->rightEdit->verticalScrollBar()->value() != value)
                {
                    ui->rightEdit->verticalScrollBar()->setValue(value);
                }
            });
    m_vRight2Left = connect(ui->rightEdit->verticalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->leftEdit->verticalScrollBar()->value() != value)
                {
                    ui->leftEdit->verticalScrollBar()->setValue(value);
                }
            });
    m_hLeft2Right = connect(ui->leftEdit->horizontalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->rightEdit->horizontalScrollBar()->value() != value)
                {
                    ui->rightEdit->horizontalScrollBar()->setValue(value);
                }
            });
    m_hRight2Left = connect(ui->rightEdit->horizontalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->leftEdit->horizontalScrollBar()->value() != value)
                {
                    ui->leftEdit->horizontalScrollBar()->setValue(value);
                }
    });
}

void MainWindow::disconnectScroll()
{
    disconnect(m_vLeft2Right);
    disconnect(m_vRight2Left);
    disconnect(m_hLeft2Right);
    disconnect(m_hRight2Left);
}

void MainWindow::initTextFormat()
{
    m_equalFormat.setBackground(QBrush(Qt::white));
    m_addFormat.setBackground(QBrush(QColor(0xc8, 0xe6, 0xc9)));
    m_removeFormat.setBackground(QBrush(QColor(0xff, 0xcd, 0xd2)));
    m_replaceFormat.setBackground(QBrush(QColor(0xff, 0xf9, 0xc4)));
    m_emptyFormat.setBackground(QBrush(QColor(0xe0, 0xe0, 0xe0)));
}

void MainWindow::setDiffLines2Edit(const std::vector<std::shared_ptr<DiffLine> > &lines, QPlainTextEdit *edit)
{
    edit->clear();
    QTextCursor cursor(edit->document());

    cursor.beginEditBlock();

    int length = static_cast<int>(lines.size());
    for (int i = 0; i < length; i++)
    {
        auto &sp = lines[i];
        switch(sp->type())
        {
        case DiffLine::Empty:
            cursor.setBlockFormat(m_emptyFormat);
            break;
        case DiffLine::Equal:
            cursor.setBlockFormat(m_equalFormat);
            cursor.insertText(*(sp->string()));
            break;
        case DiffLine::Add:
            cursor.setBlockFormat(m_addFormat);
            cursor.insertText(*(sp->string()));
            break;
        case DiffLine::Remove:
            cursor.setBlockFormat(m_removeFormat);
            cursor.insertText(*(sp->string()));
            break;
        case DiffLine::Replace:
            assert(false);
            break;
        default:
            assert(false);
            break;
        }
        if (i != length - 1)
        {
            cursor.insertText(TEXTEDIT_DELIMITER);
        }
    }

    cursor.endEditBlock();
}
