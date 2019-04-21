#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myersdiff.h"
#include "editdistance.h"
#include "linediff.h"
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

#ifdef PROFILE
    QFile fromFile("../test/benchmark2a.txt");
    QFile toFile("../test/benchmark2b.txt");

    fromFile.open(QIODevice::ReadOnly | QIODevice::Text);
    toFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QString fromList = fromFile.readAll();
    QString toList = toFile.readAll();

    ui->leftEdit->setPlainText(fromList);
    ui->rightEdit->setPlainText(toList);

    on_diffButton_clicked();
    exit(0);
#endif
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

void MainWindow::on_onlyDiff_toggled(bool checked)
{
    if (checked)
    {
        ui->preferLabel->setEnabled(false);
        ui->qualityLabel->setEnabled(false);
        ui->speedLabel->setEnabled(false);
        ui->preferSlider->setEnabled(false);
    }
    else
    {
        ui->preferLabel->setEnabled(true);
        ui->qualityLabel->setEnabled(true);
        ui->speedLabel->setEnabled(true);
        ui->preferSlider->setEnabled(true);
    }
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
    ui->leftEdit->setInDiffMode(true);
    ui->rightEdit->setInDiffMode(true);
}

void MainWindow::enterEdit()
{
    ui->leftEdit->clear();
    ui->rightEdit->clear();

    QTextCursor leftCursor(ui->leftEdit->document());
    QTextCursor rightCursor(ui->rightEdit->document());

    leftCursor.setCharFormat(m_equalCharFormat);
    rightCursor.setCharFormat(m_equalCharFormat);

    leftCursor.insertText(m_leftPlainText);
    rightCursor.insertText(m_rightPlainText);

    ui->leftEdit->setInDiffMode(false);
    ui->rightEdit->setInDiffMode(false);
}

void MainWindow::doDiff()
{
    QStringList fromList = m_leftPlainText.split(STRING_DELIMITER);
    QStringList toList = m_rightPlainText.split(STRING_DELIMITER);

    QTime timer;

    timer.start();
    auto diffText = calculateDiff(fromList, toList);
    lineDiff(diffText);
    int diffTime = timer.elapsed();

    timer.restart();
    showDiff(diffText);
    int showTime = timer.elapsed();

    statusBar()->showMessage(QString("%1 diff time: %2s  show time: %3s").arg(statusBar()->currentMessage()).arg(diffTime * 0.001).arg(showTime * 0.001));
}

DiffText<QString> MainWindow::calculateDiff(const QStringList &fromList, const QStringList &toList)
{
    if (ui->onlyDiff->isChecked())
    {
        return myersDiff(fromList, toList);
    }
    else
    {
        int preferValue = ui->preferSlider->value();
        if (preferValue == 0)
        {
            return editDistance(fromList, toList, lcsCost<QString>);
        }
        else if (preferValue == 1)
        {
            return myersDiff(fromList, toList, lcsCompare<QString>);
        }
        else if (preferValue == 2)
        {
            return myersDiff(fromList, toList, affixCompare<QString>);
        }
        else
        {
            // this won't happen
            return DiffText<QString>();
        }
    }
}

void MainWindow::showDiff(const DiffText<QString> &text)
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

    m_equalCharFormat.setBackground(QBrush(Qt::transparent));
    m_addCharFormat.setBackground(QBrush(QColor(0xa5, 0xd6, 0xa7)));
    m_removeCharFormat.setBackground(QBrush(QColor(0xef, 0x9a, 0x9a)));
}

void MainWindow::setDiffLines2Edit(const std::vector<std::shared_ptr<DiffLine<QString>> > &lines, TextEdit *edit)
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
        case LineType::Empty:
            cursor.setBlockFormat(m_emptyFormat);
            break;
        case LineType::Equal:
            cursor.setBlockFormat(m_equalFormat);
            cursor.insertText(*(sp->string()));
            break;
        case LineType::Add:
            cursor.setBlockFormat(m_addFormat);
            cursor.insertText(*(sp->string()));
            break;
        case LineType::Remove:
            cursor.setBlockFormat(m_removeFormat);
            cursor.insertText(*(sp->string()));
            break;
        case LineType::Replace:
            cursor.setBlockFormat(m_replaceFormat);
            setReplaceLine(cursor, sp);
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

    std::vector<int> blockRealLineNumber(length);
    int lineNumber = 1;
    for (int i = 0; i < length; i++)
    {
        auto &sp = lines[i];
        if (sp->type() != LineType::Empty)
        {
            blockRealLineNumber[i] = lineNumber;
            lineNumber++;
        }
        else
        {
            blockRealLineNumber[i] = -1;
        }
    }

    edit->setBlockRealLineNumber(blockRealLineNumber);
}

void MainWindow::setReplaceLine(QTextCursor &cursor, const std::shared_ptr<DiffLine<QString> > &line)
{
    auto subSection = line->subSection();
    auto subType = line->subType();
    int subLen = subSection->size();

    auto str = line->string();

    // ignore the LineType::Invalid in beginning
    for (int i = 1; i < subLen; i++)
    {
        auto curType = subType->at(i);
        int beginIndex = subSection->at(i);
        int endIndex;
        if (i == subLen - 1)
        {
            endIndex = str->size();
        }
        else
        {
            endIndex = subSection->at(i + 1);
        }
        if (curType == LineType::Equal)
        {
            cursor.setCharFormat(m_equalCharFormat);
        }
        else if (curType == LineType::Add)
        {
            cursor.setCharFormat(m_addCharFormat);
        }
        else if (curType == LineType::Remove)
        {
            cursor.setCharFormat(m_removeCharFormat);
        }
        else
        {
            assert(false);
        }

        cursor.insertText(str->mid(beginIndex, endIndex - beginIndex));
    }
    cursor.setCharFormat(m_equalCharFormat);
}
