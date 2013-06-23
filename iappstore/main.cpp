//
//  main.cpp
//  iappstore
//
//  Created by Ethan Laur on 6/23/13.
//  Copyright (c) 2013 Ethan Laur. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curl/curl.h>

using namespace std;

char list_location[60];

typedef struct app_s
{
    char appname[30];
    char location[50];
    char developer[30];
    char description[150];
    int version;
    bool setup;
    app_s *next;
    app_s *parent;
} app;

app *first = new app;
app *current = first;

void curl_download(char * url, char * out)
{
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    curl = curl_easy_init();
    if(curl)
    {
        outfile = fopen(out, "w");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FILE, outfile);
        
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        /* always cleanup */
        curl_easy_cleanup(curl);
        fflush(outfile);
        fclose(outfile);
    }
}

void downloadlist()
{
    srand((unsigned int)time(0));
    sprintf(list_location, "%s%d", "/tmp/iapp-", rand());
    char command[200];
    sprintf(command, "mkdir -p %s", list_location);
    system(command);
    sprintf(command, "%s/list.txt", list_location);
    //system(command);
    curl_download("http://iappstore.co.nf/list.txt", command);
    //execle("git", "clone", "http://github.com/phyrrus9/apps", list_location);
}

void parselist()
{
    char file[80];
    FILE *f;
    sprintf(file, "%s/list.txt", list_location);
    //ifstream f(file, ios::in);
    f = fopen(file, "r");
    if (!f)
        cout << "Error reading list!" << endl;
    int file_return = 0;
    app *tmp = first;
    char whitespace[30];
    while (file_return != EOF)
    {
        file_return = fscanf(f, "%s", tmp->appname);
        if (file_return == EOF)
            continue;
        fscanf(f, "%s", tmp->location);
        fscanf(f, "%s", tmp->developer);
        fscanf(f, "%d", &tmp->version);
        fscanf(f, "%[^#]s", tmp->description);
        file_return = fscanf(f, "%s", whitespace);
        if (file_return == EOF)
            continue;
        app *next = new app;
        tmp->next = next;
        tmp->next->parent = tmp;
        tmp = next;
    }
    tmp->parent->next = NULL;
    current = first;
}

void printlist(app * start)
{
    app *test = start;
    if (!start)
    {
        cout << "Nothing to see here" << endl;
        return;
    }
    while (test != NULL)
    {
        printf("\nName: %s\n"
               "-->Location: %s\n"
               "-->Developer: %s\n"
               "-->Description: %s\n",
               test->appname,
               test->location,
               test->developer,
               test->description);
        test = test->next;
    }
}

app * search(const char * str, bool nameonly = false)
{
    app * ret = new app;
    app * rfirst = ret;
    app * list = first;
    while (list != NULL)
    {
        bool good = false;
        if (nameonly)
        {
            if (strcmp(list->appname, str) == 0)
            {
                good = true;
                ret->setup = true;
            }
        }
        else
        {
            if (strstr(list->description, str) != NULL || strstr(list->appname, str) != NULL || strstr(list->developer, str) != NULL)
            {
                ret->setup = true;
                good = true;
            }
        }
        if (good)
        {
            if (ret == NULL)
                ret = new app;
            strcpy(ret->appname, list->appname);
            strcpy(ret->location, list->location);
            strcpy(ret->developer, list->developer);
            strcpy(ret->description, list->description);
            ret->version = list->version;
            list = list->next;
            if (!list)
                continue;
            ret->next = new app;
            ret->next->parent = ret;
            ret = ret->next;
        }
        else
            list = list->next;
    }
    if (!ret->setup)
    {
        if (ret == rfirst)
        {
            ret = NULL;
        }
        else
        {
            ret->parent->next = NULL;
            ret = rfirst;
        }
    }
    return ret;
}

void install(app * applist, char * location = (char *)"~/Desktop/iappstore")
{
    char command[100];
    char filename[50];
    sprintf(command, "mkdir -p %s", location);
    system(command);
    while (applist)
    {
        sprintf(filename, "%s.ipa", applist->appname);
        sprintf(command, "cp %s/%s %s/", list_location, filename, location);
        system(command);
        applist = applist->next;
    }
}

app * fetch_packages(app *list)
{
    char URL[70];
    char FNM[40];
    app *a = list;
    while (a)
    {
        sprintf(FNM, "%s/%s.ipa", list_location, a->appname);
        sprintf(URL, "%s", a->location);
        curl_download(URL, FNM);
        a = a->next;
    }
    a = list;
    return a; //returns original list
}

int main(int argc, const char * argv[])
{
    if (argc < 2)
    {
        cout << "Error, must specify at least one arg" << endl;
        return -1;
    }
   // if (strcmp(argv[1], "update") == 0)
    {
        downloadlist();
        parselist();
    }
    if (strcmp(argv[1], "search") == 0)
    {
        for (int i = 2; i < argc; i++)
        {
            printlist(search(argv[i]));
        }
    }
    else if (strcmp(argv[1], "install") == 0)
    {
        for (int i = 2; i < argc; i++)
        {
            cout << "Installing " << argv[i] << endl;
            install(fetch_packages(search(argv[i])));
        }
    }
    else
        cout << "idk what to do..." << endl;
}

