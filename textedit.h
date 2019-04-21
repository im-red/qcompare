#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QResizeEvent>

#include <vector>

class LineNumberWidget;

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    TextEdit(QWidget *parent = nullptr);
    void paintLineNumber(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setInDiffMode(bool isInDiffMode) { m_isInDiffMode = isInDiffMode; }
    void setBlockRealLineNumber(const std::vector<int> &v) { m_blockRealLineNumber = v; }

public slots:
    void updateLineNumberMargin();
    void updateLineNumberArea(const QRect &r, int dy);

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    LineNumberWidget *m_lineNumberWidget;
    bool m_isInDiffMode;
    std::vector<int> m_blockRealLineNumber;
    int m_lineNumberRightPadding;
};

class LineNumberWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberWidget(TextEdit *textWidget = nullptr)
        : QWidget(textWidget)
    {
        m_textWidget = textWidget;
        setAutoFillBackground(true);
    }

protected:
    QSize sizeHint() const override
    {
        return QSize(m_textWidget->lineNumberAreaWidth(), 0);
    }

    void paintEvent(QPaintEvent *e) override
    {
        m_textWidget->paintLineNumber(e);
    }

private:
    TextEdit *m_textWidget;
};

#endif // TEXTEDIT_H
