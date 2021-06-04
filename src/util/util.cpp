//------------------------------------------------------------------
// util.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------
#include "util.h"
#include "inkview.h"

#include <string>
#include <math.h>
#include <curl/curl.h>
#include <tuple>

#include <signal.h>

pid_t child_pid = -1; //Global


using std::string;

size_t Util::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

size_t Util::writeData(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = iv_fwrite(ptr, size, nmemb, stream);
    return written;
}

//https://github.com/pmartin/pocketbook-demo/blob/master/devutils/wifi.cpp
bool Util::connectToNetwork()
{
    iv_netinfo *netinfo = NetInfo();
    if (netinfo->connected)
        return true;

    const char *network_name = nullptr;
    int result = NetConnect2(network_name, 1);
    if (result != 0)
    {
        return false;
    }

    netinfo = NetInfo();
    if (netinfo->connected)
        return true;

    return false;
}

int Util::progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    std::ignore = ultotal;
    std::ignore = ulnow;
    free(clientp);
    if (dltotal <= 0.0)
        return 0;

    int percentage = round(dlnow / dltotal * 100);
    if (percentage % 10 == 0)
        UpdateProgressbar("Downloading file", percentage);

    return 0;
}

string Util::getXMLAttribute(const string &buffer, const string &name)
{
    string returnString = buffer;
    string searchString = "<" + name + ">";
    size_t found = buffer.find(searchString);

    if (found != std::string::npos)
    {
        returnString = returnString.substr(found + searchString.length());

        return returnString.substr(0, returnString.find("</" + name + ">"));
    }

    return NULL;
}

void Util::decodeUrl(string &text)
{
    char *buffer;
    CURL *curl = curl_easy_init();

    buffer = curl_easy_unescape(curl,text.c_str(),0,NULL);
    text =  buffer;

    curl_free(buffer);
    curl_easy_cleanup(curl);
}

void kill_child(int sig)
{
    //SIGKILL
    kill(child_pid, SIGTERM);
}

void Util::updatePBLibrary()
{
    //TODO how determine time?
    UpdateProgressbar("Updating PB library", 50);
    //https://stackoverflow.com/questions/6501522/how-to-kill-a-child-process-by-the-parent-process
    signal(SIGALRM, (void (*)(int))kill_child);
    child_pid = fork();
    if (child_pid > 0)
    {
        //parent
        alarm(5);
        wait(NULL);
    }
    else if (child_pid == 0)
    {
        //child
        string cmd = "/ebrmain/bin/scanner.app";
        execlp(cmd.c_str(), cmd.c_str(), (char *)NULL);
    }
}