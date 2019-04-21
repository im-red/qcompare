#include "textedit.h"
#include <QDebug>
#include <QPainter>
#include <QTextBlock>

TextEdit::TextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberWidget(nullptr)
    , m_isInDiffMode(false)
{
    m_lineNumberWidget = new LineNumberWidget(this);

    connect(this, &TextEdit::blockCountChanged, this, &TextEdit::updateLineNumberMargin);
    connect(this, static_cast<void(TextEdit::*)(const QRect &, int)>(&TextEdit::updateRequest), this, &TextEdit::updateLineNumberArea);

    updateLineNumberMargin();
}

void TextEdit::paintLineNumber(QPaintEvent *event)
{
    QPainter painter(m_lineNumberWidget);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number("");
            if (m_isInDiffMode)
            {
                int realLineNumber = m_blockRealLineNumber[blockNumber];
                if (realLineNumber != -1)
                {
                    number = QString::number(realLineNumber);
                }
            }
            else
            {
                number = QString::number(blockNumber + 1);
            }
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberWidget->width() - m_lineNumberRightPadding, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void TextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    m_lineNumberWidget->setGeometry(frameWidth(), frameWidth(),
                                    lineNumberAreaWidth(), height() - 2 * frameWidth());
}

int TextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }
    digits++;

    int singleCharWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    int space = 3 + singleCharWidth * digits;
    m_lineNumberRightPadding = singleCharWidth / 2;
    return space;
}

void TextEdit::updateLineNumberMargin()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEdit::updateLineNumberArea(const QRect &r, int dy)
{
    if (dy)
    {
        m_lineNumberWidget->scroll(0, dy);
    }
    else
    {
        m_lineNumberWidget->update(0, r.y(), lineNumberAreaWidth(), r.height());
    }
    if (r.contains(viewport()->rect()))
    {
        updateLineNumberMargin();
    }
}
