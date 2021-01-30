/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <vector>
#include <dirent.h>
#include <unistd.h>

using namespace std;

vector<int> getProcIdByName(string procName)
{
    vector<int> pids;

    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while ((dirp = readdir(dp)) != NULL)
        {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                string cmdPath = string("/proc/") + dirp->d_name + "/cmdline";
                ifstream cmdFile(cmdPath.c_str(), ifstream::in);
                string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                  // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(0, pos);
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(pos + 1);
                    // Compare against requested process name
                    if (procName == cmdLine)
                        pids.push_back(id);
                }
            }
        }
    }

    closedir(dp);

    return pids;
}

void KillProgramsByName(string sName) {
    vector<int> vId = getProcIdByName(sName);
    int nSelfId = getpid();
    char sCommand[256];
    for(vector<int>::const_iterator i = vId.begin(); i != vId.end(); ++i) {
        if(*i != nSelfId) {
            sprintf(sCommand, "kill -9 %d", *i);
            system(sCommand);
        }
    }
}

string GetProgName(const string & sPath) {
    auto pos = sPath.find_last_of("/");
    if(pos == string::npos)
        return sPath;
    return sPath.substr(pos + 1);
}

int main(int argc, char *argv[])
{
    KillProgramsByName(GetProgName(argv[0]));
    // This is required for tesseract
    qputenv("LC_ALL", "C");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
