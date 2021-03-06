/*
 * Copyright (C) Pedram Pourang (aka Tsu Jan) 2014 <tsujan2000@gmail.com>
 *
 * FeatherPad is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FeatherPad is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "singleton.h"
#include "x11.h"
#include <signal.h>

void handleQuitSignals (const std::vector<int>& quitSignals)
{
    auto handler = [](int sig) ->void {
        Q_UNUSED (sig);
        //printf("\nUser signal = %d.\n", sig);
        QCoreApplication::quit();
    };

    for (int sig : quitSignals )
        signal (sig, handler); // handle these signals by quitting gracefully
}

int main (int argc, char *argv[])
{
    FeatherPad::FPsingleton singleton (argc, argv, QString (qgetenv ("USER")) + "-featherpad");

    handleQuitSignals ({SIGQUIT, SIGINT, SIGTERM, SIGHUP}); // -> https://en.wikipedia.org/wiki/Unix_signal

#if QT_VERSION >= 0x050500
    singleton.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    QStringList langs (QLocale::system().uiLanguages());
    QString lang; // bcp47Name() doesn't work under vbox
    if (!langs.isEmpty())
        lang = langs.first().split('-').first();

    QTranslator qtTranslator;
    qtTranslator.load ("qt_" + lang, QLibraryInfo::location (QLibraryInfo::TranslationsPath));
    singleton.installTranslator (&qtTranslator);

    QTranslator FPTranslator;
    FPTranslator.load ("featherpad_" + lang, DATADIR "/featherpad/translations");
    singleton.installTranslator (&FPTranslator);

    QString info;
    int d = FeatherPad::fromDesktop();
    info.setNum (d);
    info += "\n\r"; // a string that can't be used in file names
    for (int i = 1; i < argc; ++i)
    {
        info += QString::fromUtf8 (argv[i]);
        if (i < argc - 1)
            info += "\n\r";
    }

    // the slot is executed in the receiver's thread
    if (singleton.sendMessage (info))
        return 0;

    //QObject::connect (&singleton, SIGNAL(aboutToQuit()), &singleton, SLOT(quitting()));
    singleton.newWin (info);
    QObject::connect (&singleton, &FeatherPad::FPsingleton::messageReceived,
                      &singleton, &FeatherPad::FPsingleton::handleMessage);

    return singleton.exec();
}
