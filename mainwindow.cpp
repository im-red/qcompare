#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editdistance.h"
#include <QDebug>
#include <QAbstractSlider>
#include <QScrollBar>
#include <utility>

const QChar MainWindow::TEXTEDIT_DELIMITER = QChar(0x2029);
const QChar MainWindow::MANUAL_DELIMITER = QChar(0x2029);
const QChar MainWindow::ACTUAL_DELIMITER = '\n';
const QChar MainWindow::ESCAPE_CHAR = QChar(0x0);

static std::map<EditOperation, QString> Operation2String
{
    { Nop, "Nop" },
    { Add, "Add" },
    { Remove, "Remove" },
    { Replace, "Replace" },
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_isShowing(false)
{
    ui->setupUi(this);

    initWidget();
    initConnection();
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

    enterDiff();
}

void MainWindow::on_editButton_clicked()
{
    ui->diffButton->setChecked(false);
    ui->diffButton->setEnabled(true);
    ui->editButton->setEnabled(false);

    ui->leftEdit->setReadOnly(false);
    ui->rightEdit->setReadOnly(false);

    enterEdit();
}

void MainWindow::enterDiff()
{
    QString leftText = convertFromRawToPlain(ui->leftEdit->document()->toRawText());
    qDebug() << "left:" << leftText;
    if (leftText != m_leftPlainText)
    {
        m_leftPlainText = std::move(leftText);
    }

    QString rightText = convertFromRawToPlain(ui->rightEdit->document()->toRawText());
    qDebug() << "right:" << rightText;
    if (rightText != m_rightPlainText)
    {
        m_rightPlainText = rightText;
    }

    doDiff();
}

void MainWindow::enterEdit()
{
    ui->leftEdit->setPlainText(m_leftPlainText);
    ui->rightEdit->setPlainText(m_rightPlainText);
}

QString MainWindow::convertFromRawToPlain(const QString &raw)
{
    QString result;
    result.reserve(raw.size());

    QChar prevChar(0);

    int rawLen = raw.size();
    for (int i = 0; i < rawLen; i++)
    {
        QChar c = raw[i];

        if (c == TEXTEDIT_DELIMITER)
        {
            result += ACTUAL_DELIMITER;
        }
        else
        {
            result += c;
        }

        prevChar = c;
    }
    return result;
}

void MainWindow::doDiff()
{
    QStringList fromList = m_leftPlainText.split(ACTUAL_DELIMITER);
    QStringList toList = m_rightPlainText.split(ACTUAL_DELIMITER);

    qDebug() << "----------";
    qDebug() << fromList;
    qDebug() << toList;
    qDebug() << "----------";

    auto ops = articleEditOperations(fromList, toList);
    for (auto op : ops)
    {
        qDebug() << Operation2String[op];
    }

    showDiff(ops, fromList, toList);
}

void MainWindow::showDiff(const std::vector<EditOperation> &ops, const QStringList &fromList, const QStringList &toList)
{
    m_isShowing = true;

    int fromIndex = 0;
    int toIndex = 0;

    m_leftRawText.clear();
    m_rightRawText.clear();

    m_leftRawText.reserve(m_leftPlainText.size() + ops.size());
    m_rightRawText.reserve(m_rightPlainText.size() + ops.size());

    ui->leftEdit->clear();
    ui->rightEdit->clear();

    QTextCursor leftCursor(ui->leftEdit->document());
    QTextCursor rightCursor(ui->rightEdit->document());

    for (auto op : ops)
    {
        if (op == Nop)
        {
            leftCursor.setBlockFormat(m_nopFormat);
            leftCursor.insertText(fromList[fromIndex]);

            leftCursor.insertText(TEXTEDIT_DELIMITER);
            leftCursor.setBlockFormat(m_nopFormat);

            rightCursor.setBlockFormat(m_nopFormat);
            rightCursor.insertText(toList[toIndex]);

            rightCursor.insertText(TEXTEDIT_DELIMITER);
            rightCursor.setBlockFormat(m_nopFormat);

            fromIndex++;
            toIndex++;
        }
        else if (op == Add)
        {
            leftCursor.setBlockFormat(m_emptyFormat);
            leftCursor.insertText(MANUAL_DELIMITER);
            leftCursor.setBlockFormat(m_nopFormat);

            rightCursor.setBlockFormat(m_addFormat);
            rightCursor.insertText(toList[toIndex]);

            rightCursor.insertText(TEXTEDIT_DELIMITER);
            rightCursor.setBlockFormat(m_nopFormat);

            toIndex++;
        }
        else if (op == Remove)
        {
            leftCursor.setBlockFormat(m_removeFormat);
            leftCursor.insertText(fromList[fromIndex]);

            leftCursor.insertText(TEXTEDIT_DELIMITER);
            leftCursor.setBlockFormat(m_nopFormat);

            rightCursor.setBlockFormat(m_emptyFormat);
            rightCursor.insertText(MANUAL_DELIMITER);
            rightCursor.setBlockFormat(m_nopFormat);

            fromIndex++;
        }
        else // op == replace
        {
            leftCursor.setBlockFormat(m_replaceFormat);
            leftCursor.insertText(fromList[fromIndex]);

            leftCursor.insertText(TEXTEDIT_DELIMITER);
            leftCursor.setBlockFormat(m_nopFormat);

            rightCursor.setBlockFormat(m_replaceFormat);
            rightCursor.insertText(toList[toIndex]);

            rightCursor.insertText(TEXTEDIT_DELIMITER);
            rightCursor.setBlockFormat(m_nopFormat);

            fromIndex++;
            toIndex++;
        }
    }

    m_isShowing = false;
}

void MainWindow::initWidget()
{
    ui->leftEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    ui->rightEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

    ui->leftEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->diffButton->setCheckable(true);
    ui->diffButton->setChecked(false);
    ui->editButton->setCheckable(true);
    ui->editButton->setChecked(true);
    ui->editButton->setEnabled(false);
}

void MainWindow::initConnection()
{
    connect(ui->leftEdit->verticalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->rightEdit->verticalScrollBar()->value() != value)
                {
                    ui->rightEdit->verticalScrollBar()->setValue(value);
                }
            });
    connect(ui->rightEdit->verticalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->leftEdit->verticalScrollBar()->value() != value)
                {
                    ui->leftEdit->verticalScrollBar()->setValue(value);
                }
            });
    connect(ui->leftEdit->horizontalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->rightEdit->horizontalScrollBar()->value() != value)
                {
                    ui->rightEdit->horizontalScrollBar()->setValue(value);
                }
            });
    connect(ui->rightEdit->horizontalScrollBar(), &QAbstractSlider::valueChanged,
            [this](int value)
            {
                if (ui->leftEdit->horizontalScrollBar()->value() != value)
                {
                    ui->leftEdit->horizontalScrollBar()->setValue(value);
                }
    });
}

void MainWindow::initTextFormat()
{
    m_nopFormat.setBackground(QBrush(Qt::white));
    m_addFormat.setBackground(QBrush(QColor(0xc8, 0xe6, 0xc9)));
    m_removeFormat.setBackground(QBrush(QColor(0xff, 0xcd, 0xd2)));
    m_replaceFormat.setBackground(QBrush(QColor(0xff, 0xf9, 0xc4)));
    m_emptyFormat.setBackground(QBrush(QColor(0xe0, 0xe0, 0xe0)));
}
