#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "editdistance.h"
#include "myersdiff.h"
#include <QDebug>
#include <QAbstractSlider>
#include <QScrollBar>
#include <utility>
#include <QDate>

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
{
    ui->setupUi(this);
    showMaximized();

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
    if (leftText != m_leftPlainText)
    {
        m_leftPlainText = std::move(leftText);
    }

    QString rightText = convertFromRawToPlain(ui->rightEdit->document()->toRawText());
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

    QTime timer;

    timer.start();
    auto diffText = myersDiff(fromList, toList);
    //auto ops = articleEditOperations(fromList, toList);
    int diffTime = timer.elapsed();

    timer.restart();
    showDiff(diffText);
    //showDiff(ops, fromList, toList);
    int showTime = timer.elapsed();

    statusBar()->showMessage(QString("diff time: %1s show time: %2s").arg(diffTime * 0.001).arg(showTime * 0.001));
}

void MainWindow::showDiff(const std::vector<EditOperation> &ops, const QStringList &fromList, const QStringList &toList)
{
    int fromIndex = 0;
    int toIndex = 0;

    ui->leftEdit->clear();
    ui->rightEdit->clear();

    QTextCursor leftCursor(ui->leftEdit->document());
    QTextCursor rightCursor(ui->rightEdit->document());

    for (auto op : ops)
    {
        if (op == Nop)
        {
            leftCursor.setBlockFormat(m_equalFormat);
            leftCursor.insertText(fromList[fromIndex]);

            leftCursor.insertText(TEXTEDIT_DELIMITER);
            leftCursor.setBlockFormat(m_equalFormat);

            rightCursor.setBlockFormat(m_equalFormat);
            rightCursor.insertText(toList[toIndex]);

            rightCursor.insertText(TEXTEDIT_DELIMITER);
            rightCursor.setBlockFormat(m_equalFormat);

            fromIndex++;
            toIndex++;
        }
        else if (op == Add)
        {
            leftCursor.setBlockFormat(m_emptyFormat);
            leftCursor.insertText(MANUAL_DELIMITER);
            leftCursor.setBlockFormat(m_equalFormat);

            rightCursor.setBlockFormat(m_addFormat);
            rightCursor.insertText(toList[toIndex]);

            rightCursor.insertText(TEXTEDIT_DELIMITER);
            rightCursor.setBlockFormat(m_equalFormat);

            toIndex++;
        }
        else if (op == Remove)
        {
            leftCursor.setBlockFormat(m_removeFormat);
            leftCursor.insertText(fromList[fromIndex]);

            leftCursor.insertText(TEXTEDIT_DELIMITER);
            leftCursor.setBlockFormat(m_equalFormat);

            rightCursor.setBlockFormat(m_emptyFormat);
            rightCursor.insertText(MANUAL_DELIMITER);
            rightCursor.setBlockFormat(m_equalFormat);

            fromIndex++;
        }
        else // op == replace
        {
            leftCursor.setBlockFormat(m_replaceFormat);
            leftCursor.insertText(fromList[fromIndex]);

            leftCursor.insertText(TEXTEDIT_DELIMITER);
            leftCursor.setBlockFormat(m_equalFormat);

            rightCursor.setBlockFormat(m_replaceFormat);
            rightCursor.insertText(toList[toIndex]);

            rightCursor.insertText(TEXTEDIT_DELIMITER);
            rightCursor.setBlockFormat(m_equalFormat);

            fromIndex++;
            toIndex++;
        }
    }
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
            cursor.insertText(MANUAL_DELIMITER);
        }
    }
}
