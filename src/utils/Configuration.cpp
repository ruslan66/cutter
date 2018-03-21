#include "Configuration.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QFontDatabase>
#include <QFile>
#include <QApplication>

Configuration* Configuration::mPtr = nullptr;

Configuration::Configuration() : QObject()
{
    mPtr = this;
    loadInitial();
}

Configuration* Configuration::instance()
{
    if (!mPtr)
        mPtr = new Configuration();
    return mPtr;
}

void Configuration::loadInitial()
{
    setDarkTheme(getDarkTheme());
    setColorTheme(getCurrentTheme());
    resetToDefaultAsmOptions();
}

void Configuration::resetAll()
{
    Core()->cmd("e-");
    Core()->setSettings();
    // Delete the file so no extra configuration is in it.
    QFile settingsFile(s.fileName());
    settingsFile.remove();
    s.clear();

    loadInitial();
    emit fontsUpdated();
    Core()->triggerAsmOptionsChanged();
}

void Configuration::loadDefaultTheme()
{
    /* Load Qt Theme */
    qApp->setStyleSheet("");

    /* Images */
    logoFile = QString(":/img/cutter_plain.svg");

    /* Colors */
    // GUI
    setColor("gui.cflow",   QColor(0, 0, 0));
    setColor("gui.dataoffset", QColor(0, 0, 0));
    setColor("gui.border",  QColor(0, 0, 0));
    setColor("highlight",   QColor(210, 210, 255));
    // Windows background
    setColor("gui.background", QColor(255, 255, 255));
    // Disassembly nodes background
    setColor("gui.alt_background", QColor(245, 250, 255));
    // Custom
    setColor("gui.imports", QColor(50, 140, 255));
    setColor("gui.main", QColor(0, 128, 0));
    setColor("gui.navbar.err", QColor(255, 0, 0));
    setColor("gui.navbar.code", QColor(104, 229, 69));
    setColor("gui.navbar.str", QColor(69, 104, 229));
    setColor("gui.navbar.sym", QColor(229, 150, 69));
    setColor("gui.navbar.empty", QColor(100, 100, 100));
}

void Configuration::loadDarkTheme()
{
    /* Load Qt Theme */
    QFile f(":qdarkstyle/style.qss");
    if (!f.exists())
    {
        qWarning() << "Can't find dark theme stylesheet.";
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        QString stylesheet = ts.readAll();
#ifdef Q_OS_MACX
        // see https://github.com/ColinDuquesnoy/QDarkStyleSheet/issues/22#issuecomment-96179529
        stylesheet += "QDockWidget::title"
                "{"
                "    background-color: #31363b;"
                "    text-align: center;"
                "    height: 12px;"
                "}";
#endif
        qApp->setStyleSheet(stylesheet);
    }

    /* Images */
    logoFile = QString(":/img/cutter_white_plain.svg");

    /* Colors */
    // GUI
    setColor("gui.cflow",   QColor(255, 255, 255));
    setColor("gui.dataoffset", QColor(255, 255, 255));
    setColor("gui.border",  QColor(255, 255, 255));
    setColor("highlight", QColor(64, 115, 115));
    // Windows background
    setColor("gui.background", QColor(36, 66, 79));
    // Disassembly nodes background
    setColor("gui.alt_background", QColor(58, 100, 128));
    // Custom
    setColor("gui.imports", QColor(50, 140, 255));
    setColor("gui.main", QColor(0, 128, 0));
    setColor("gui.navbar.err", QColor(255, 0, 0));
    setColor("gui.navbar.code", QColor(104, 229, 69));
    setColor("gui.navbar.str", QColor(69, 104, 229));
    setColor("gui.navbar.sym", QColor(229, 150, 69));
    setColor("gui.navbar.empty", QColor(100, 100, 100));
}

const QFont Configuration::getFont() const
{
    QFont font = s.value("font", QFont("Inconsolata", 12)).value<QFont>();
    return font;
}

void Configuration::setFont(const QFont &font)
{
    s.setValue("font", font);
    emit fontsUpdated();
}

void Configuration::setDarkTheme(bool set)
{
    s.setValue("dark", set);
    if (set) {
        loadDarkTheme();
    } else {
        loadDefaultTheme();
    }
    emit colorsUpdated();
}

QString Configuration::getLogoFile()
{
    return logoFile;
}

/*!
 * \brief Configuration::setColor sets the local Cutter configuration color
 * \param name Color Name
 * \param color The color you want to set
 */
void Configuration::setColor(const QString &name, const QColor &color)
{
    s.setValue("colors." + name, color);
}

const QColor Configuration::getColor(const QString &name) const
{
    if (s.contains("colors." + name)) {
        return s.value("colors." + name).value<QColor>();
    } else {
        return s.value("colors.other").value<QColor>();
    }
}

void Configuration::setColorTheme(QString theme)
{
    if (theme == "default") {
        Core()->cmd("ecd");
        s.setValue("theme", "default");
    } else {
        Core()->cmd(QString("eco %1").arg(theme));
        s.setValue("theme", theme);
    }
    // Duplicate interesting colors into our Cutter Settings
    // Dirty fix for arrow colors, TODO refactor getColor, setColor, etc.
    QJsonDocument colors = Core()->cmdj("ecj");
    QJsonObject colorsObject = colors.object();
    QJsonObject::iterator it;
    for (it = colorsObject.begin(); it != colorsObject.end(); it++) {
        if (!it.key().contains("graph"))
            continue;
        QJsonArray rgb = it.value().toArray();
        s.setValue("colors." + it.key(), QColor(rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt()));
    }
    emit colorsUpdated();
}

void Configuration::resetToDefaultAsmOptions()
{
#define RESTORE_ASMOPT(option, defaultValue) { Core()->setConfig(option, getDefaultAsmOption(option, defaultValue)); }
    RESTORE_ASMOPT("asm.esil", false);
    RESTORE_ASMOPT("asm.pseudo", false);
    RESTORE_ASMOPT("asm.offset", true);
    RESTORE_ASMOPT("asm.describe", false);
    RESTORE_ASMOPT("asm.stackptr", false);
    RESTORE_ASMOPT("asm.slow", true);
    RESTORE_ASMOPT("asm.lines", true);
    RESTORE_ASMOPT("asm.fcnlines", true);
    RESTORE_ASMOPT("asm.flgoff", false);
    RESTORE_ASMOPT("asm.emu", false);
    RESTORE_ASMOPT("asm.cmt.right", true);
    RESTORE_ASMOPT("asm.varsum", false);
    RESTORE_ASMOPT("asm.bytes", false);
    RESTORE_ASMOPT("asm.size", false);
    RESTORE_ASMOPT("asm.bytespace", false);
    RESTORE_ASMOPT("asm.lbytes", true);
    RESTORE_ASMOPT("asm.nbytes", 10);
    RESTORE_ASMOPT("asm.syntax", QString("intel"));
    RESTORE_ASMOPT("asm.ucase", false);
    RESTORE_ASMOPT("asm.bbline", false);
    RESTORE_ASMOPT("asm.capitalize", false);
    RESTORE_ASMOPT("asm.varsub", true);
    RESTORE_ASMOPT("asm.varsub_only", true);
    RESTORE_ASMOPT("asm.tabs", 5);
#undef RESTORE_ASMOPT
}

void Configuration::saveDefaultAsmOptions()
{
    setDefaultAsmOption("asm.esil",         Core()->getConfigb("asm.esil"));
    setDefaultAsmOption("asm.pseudo",       Core()->getConfigb("asm.pseudo"));
    setDefaultAsmOption("asm.offset",       Core()->getConfigb("asm.offset"));
    setDefaultAsmOption("asm.describe",     Core()->getConfigb("asm.describe"));
    setDefaultAsmOption("asm.stackptr",     Core()->getConfigb("asm.stackptr"));
    setDefaultAsmOption("asm.slow",         Core()->getConfigb("asm.slow"));
    setDefaultAsmOption("asm.lines",        Core()->getConfigb("asm.lines"));
    setDefaultAsmOption("asm.fcnlines",     Core()->getConfigb("asm.fcnlines"));
    setDefaultAsmOption("asm.flgoff",       Core()->getConfigb("asm.flgoff"));
    setDefaultAsmOption("asm.emu",          Core()->getConfigb("asm.emu"));
    setDefaultAsmOption("asm.cmt.right",    Core()->getConfigb("asm.cmt.right"));
    setDefaultAsmOption("asm.varsum",       Core()->getConfigb("asm.varsum"));
    setDefaultAsmOption("asm.bytes",        Core()->getConfigb("asm.bytes"));
    setDefaultAsmOption("asm.size",         Core()->getConfigb("asm.size"));
    setDefaultAsmOption("asm.bytespace",    Core()->getConfigb("asm.bytespace"));
    setDefaultAsmOption("asm.lbytes",       Core()->getConfigb("asm.lbytes"));
    setDefaultAsmOption("asm.nbytes",       Core()->getConfigi("asm.nbytes"));
    setDefaultAsmOption("asm.syntax",       Core()->getConfig("asm.syntax"));
    setDefaultAsmOption("asm.ucase",        Core()->getConfigb("asm.ucase"));
    setDefaultAsmOption("asm.bbline",       Core()->getConfigb("asm.bbline"));
    setDefaultAsmOption("asm.capitalize",   Core()->getConfigb("asm.capitalize"));
    setDefaultAsmOption("asm.varsub",       Core()->getConfigb("asm.varsub"));
    setDefaultAsmOption("asm.varsub_only",  Core()->getConfigb("asm.varsub_only"));
    setDefaultAsmOption("asm.tabs",         Core()->getConfigi("asm.tabs"));
}
