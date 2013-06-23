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

using namespace std;

char list_location[60];

typedef struct app_s
{
    char appname[30];
    char location[50];
    char developer[30];
    char description[150];
    int version;
    app_s *next;
    app_s *parent;
} app;

app *first = new app;
app *current = first;

void downloadlist()
{
    srand((unsigned int)time(0));
    sprintf(list_location, "%s%d", "/tmp/iapp-", rand());
    char command[200];
    sprintf(command, "git clone -q http://github.com/phyrrus9/apps %s", list_location);
    system(command);
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
    while (test != NULL)
    {
        printf("Name: %s\n"
               "Location: %s\n"
               "Developer: %s\n"
               "Description: %s\n",
               test->appname,
               test->location,
               test->developer,
               test->description);
        test = test->next;
    }
}

app * search(char * str, bool nameonly = false)
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
            }
        }
        else
        {
            if (strstr(list->description, str) != NULL || strstr(list->appname, str) != NULL || strstr(list->developer, str) != NULL)
            {
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
    ret = rfirst;
    return ret;
}

void install(char * appname, char * location = (char *)"~/Desktop/iappstore")
{
    char command[100];
    sprintf(command, "mkdir -p %s", location);
    system(command);
    app *applocation = search(appname, true);
    if (!applocation)
        cout << "Can not find app, the printer is on fire" << endl;
    sprintf(command, "cp %s/%s %s/", list_location, applocation->location, location);
    system(command);
}

int main(int argc, const char * argv[])
{
    downloadlist();
    parselist();
    printlist(first);
    install((char *)"magicount");
}

