/*
 * This file is part of the Novile Editor
 * This program is free software licensed under the GNU LGPL. You can
 * find a copy of this license in LICENSE in the top directory of
 * the source code.
 *
 * Copyright 2013    Illya Kovalevskyy   <illya.kovalevskyy@gmail.com>
 *
 */

#include <QtCore>
#include <QVBoxLayout>
#include <QtWebKitWidgets>
#include "novile_debug.h"
#include "editor.h"

namespace Novile
{

/**
 * @brief The EditorPrivate class
 *
 * EditorPrivate class represents low-level wrapper for Ace editor and background of @see Editor
 */
class EditorPrivate: public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Regular constructor
     * @param p editor object (will be used like Q-pointer)
     */
    EditorPrivate(Editor *p = 0) :
        QObject(),
        parent(p),
        aceView(new QWebView(p)),
        layout(new QVBoxLayout(p))
    {
        parent->setLayout(layout);
        layout->addWidget(aceView);
        layout->setMargin(0);

        connect(this, &EditorPrivate::linesChanged,
                parent, &Editor::linesChanged);

        connect(this, &EditorPrivate::textChanged,
                parent, &Editor::textChanged);
    }

    ~EditorPrivate()
    {
    }

    /**
     * @brief Run some JS code to Ace
     * @param code javascript source
     * @return evaluation result
     */
    QVariant executeJavaScript(const QString &code)
    {
        return aceView->page()->mainFrame()->evaluateJavaScript(code);
    }

    /**
     * @brief Start Ace web widget and load javascript low-level helpers
     */
    void startAceWidget()
    {
        QEventLoop loop(parent);
        QObject::connect(aceView, &QWebView::loadFinished,
                         &loop, &QEventLoop::quit);
        aceView->load(QUrl("qrc:/html/ace.html"));
        loop.exec();

        // Wrapper (data/wrapper.js)
        QWebFrame *frame = aceView->page()->mainFrame();
        frame->addToJavaScriptWindowObject("Novile", this);

        QFile listeners(":/html/wrapper.js");
        if (listeners.open(QIODevice::ReadOnly))
            executeJavaScript(listeners.readAll());
    }

public slots:
    /**
     * @brief Provider for @see linesChanged()
     * @param lines new number of rows
     */
    void onLinesChanged(int lines)
    {
        emit linesChanged(lines);
    }

    /**
     * @brief Provider for @see textChanged()
     */
    void onTextChanged()
    {
        emit textChanged();
    }

signals:
    /**
     * @brief Intermediate signal for @see Editor::linesChanged()
     */
    void linesChanged(int);

    /**
     * @brief Intermediate signal for @see Editor::textChanged()
     */
    void textChanged();

public:
    Editor *parent;
    QWebView *aceView;
    QVBoxLayout *layout;
};

Editor::Editor(QWidget *parent) :
    QWidget(parent),
    d(new EditorPrivate(this))
{
    d->startAceWidget();
}

Editor::~Editor()
{
}

int Editor::lines() const
{
    return d->executeJavaScript("property('lines')").toInt();
}

void Editor::gotoLine(int lineNumber) const
{
    d->executeJavaScript("editor.gotoLine("+QString::number(lineNumber)+")");
}

QString Editor::text() const
{
    return d->executeJavaScript("property('text')").toString();
}

void Editor::setText(const QString &newText)
{
    d->executeJavaScript("editor.setValue('"+newText+"')");
    d->executeJavaScript("editor.selection.clearSelection()");
}

bool Editor::isReadOnly() const
{
    return d->executeJavaScript("property('readonly')").toBool();
}

void Editor::setReadOnly(bool readOnly)
{
    if (readOnly) {
        d->executeJavaScript("property('readonly', true);"
                             "editor.setReadOnly(true);");
    } else {
        d->executeJavaScript("property('readonly', false)"
                             "editor.setReadOnly(false);");
    }
}

void Editor::setHighlightMode(HighlightMode mode)
{
    switch(mode) {
    case ModeCpp:
        setHighlightMode(QString("c_cpp"), QUrl("qrc:/ace/mode-c_cpp.js"));
        return;
    case ModeCss:
        setHighlightMode(QString("css"), QUrl("qrc:/ace/mode-css.js"));
        return;
    case ModeHtml:
        setHighlightMode(QString("html"), QUrl("qrc:/ace/mode-html.js"));
        return;
    case ModeJavaScript:
        setHighlightMode(QString("javascript"), QUrl("qrc:/ace/mode-javascript.js"));
        return;
    case ModePascal:
        setHighlightMode(QString("pascal"), QUrl("qrc:/ace/mode-pascal.js"));
        return;
    case ModePhp:
        setHighlightMode(QString("php"), QUrl("qrc:/ace/mode-php.js"));
        return;
    case ModePython:
        setHighlightMode(QString("python"), QUrl("qrc:/ace/mode-python.js"));
        return;
    case ModeRuby:
        setHighlightMode(QString("ruby"), QUrl("qrc:/ace/mode-ruby.js"));
        return;
    case ModeXml:
        setHighlightMode(QString("xml"), QUrl("qrc:/ace/mode-xml.js"));
        return;
    }
}

void Editor::setHighlightMode(const QString &name, const QUrl &url)
{
    const QString request = ""
            "$.getScript('"+url.toString()+"');"
            "editor.getSession().setMode('ace/mode/"+name+"');";

    d->executeJavaScript(request);
}

void Editor::setTheme(Theme theme)
{
    switch(theme) {
    case ThemeAmbiance:
        setTheme(QString("ambiance"), QUrl("qrc:/ace/theme-ambiance.js"));
        return;
    case ThemeMonokai:
        setTheme(QString("monokai"), QUrl("qrc:/ace/theme-monokai.js"));
        return;
    case ThemeTextMate:
        setTheme(QString("textmate"), QUrl("qrc:/ace/theme-textmate.js"));
        return;
    }
}

void Editor::setTheme(const QString &name, const QUrl &url)
{
    const QString request = ""
            "$.getScript('"+url.toString()+"');"
            "editor.setTheme('ace/theme/"+name+"');";

    d->executeJavaScript(request);
}

} // namespace Novile

#include "editor.moc"
