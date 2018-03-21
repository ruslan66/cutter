#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QSettings>
#include <QFont>
#include <Cutter.h>

#define Config() (Configuration::instance())
#define ConfigColor(x) Config()->getColor(x)

class Configuration : public QObject
{
    Q_OBJECT
private:
    QSettings s;
    static Configuration *mPtr;

    void loadInitial();

    // Colors
    void loadDefaultTheme();
    void loadDarkTheme();
    void setColor(const QString &name, const QColor &color);

    // Images
    QString logoFile;

    // AsmOptions
    bool getDefaultAsmOption(const QString &option, bool defaultValue)              { return s.value("asmoptions." + option, defaultValue).toBool(); }
    int  getDefaultAsmOption(const QString &option, int defaultValue)               { return s.value("asmoptions." + option, defaultValue).toInt(); }
    QString getDefaultAsmOption(const QString &option, const QString &defaultValue) { return s.value("asmoptions." + option, defaultValue).toString(); }
    void setDefaultAsmOption(const QString &option, const QVariant &value)          { s.setValue("asmoptions." + option, value); }

public:
    // Functions
    Configuration();
    static Configuration* instance();

    void resetAll();

    // Fonts
    const QFont getFont() const;
    void setFont(const QFont &font);

    // Colors
    const QColor getColor(const QString &name) const;
    void setDarkTheme(bool set);
    bool getDarkTheme()                 { return s.value("dark").toBool(); }

    // Images
    QString getLogoFile();

    // Asm Options
    void resetToDefaultAsmOptions();
    void saveDefaultAsmOptions();

    // Graph
    int getGraphBlockMaxChars() const   { return s.value("graph.maxcols", 50).toInt(); }
    void setGraphBlockMaxChars(int ch)  { s.setValue("graph.maxcols", ch); }

    QString getCurrentTheme() const     { return s.value("theme", "solarized").toString(); }
    void setColorTheme(QString theme);

signals:
    void fontsUpdated();
    void colorsUpdated();
};

#endif // CONFIGURATION_H
