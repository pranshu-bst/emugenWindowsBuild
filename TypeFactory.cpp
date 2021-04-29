/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#pragma warning(disable : 4996)
#include "TypeFactory.h"
#include "VarType.h"
#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include "strUtils.h"

std::string parseTypeAndSize(const std::string & f, size_t pos, size_t* last, std::string *varWidth)
{
    bool done = false;
    std::string vartype = "";

    enum { ST_TYPE, ST_NAME, ST_END } state = ST_TYPE;

    while(!done) {

        std::string str = getNextToken(f, pos, last, WHITESPACE);
        if (str.size() == 0) break;

        switch(state) {
        case ST_TYPE:
            if (str == "const") {
                pos = *last;
                vartype = "const ";
            } else {
                // must be a type name;
                vartype += str;
                state = ST_NAME;
                pos = *last;
            }
            break;
        case ST_NAME:
            if (str.size() == 0) {
                done = true;
            } else if (str == "*") {
                vartype += "*";
                pos = *last;
	    } else if (str == "const*") {
                vartype += " const*";
		pos = *last;
	    } else if (str == "const") {
                vartype += " const";
		pos = *last;
            } else {
                while (str[0] == '*') {
                    vartype += "*";
                    str[0] = ' ';
                    str = trim(str);
                }
		*varWidth = str;
                done = true;
            }
            break;
        case ST_END:
            break;
        }
    }
    return vartype;
}

std::string parsePrintString(const std::string & f, size_t pos, size_t* last)
{
    std::string vartype = "";

    std::string str = getNextToken(f, pos, last, WHITESPACE);
    if (str.size() == 0) return vartype;

    vartype  += str;
    pos = *last;
    size_t p = str.find ("\"");

    if ((p != std::string::npos) && (str.length()-1 == p)) {
        str = getNextToken(f, pos, last, WHITESPACE);
        vartype += " ";
        vartype += str;
        pos = *last;
        str = getNextToken(f, pos, last, WHITESPACE);
        vartype += " ";
        vartype += str;
        pos = *last;
    }

    return vartype;
}

TypeFactory * TypeFactory::m_instance = NULL;

static Var0 g_var0;
static Var8 g_var8;
static Var16 g_var16;
static Var32 g_var32;
static Var64 g_var64;

typedef std::map<std::string, VarType> TypeMap;
static  TypeMap g_varMap;
static bool g_initialized = false;
static int g_typeId = 0;


static VarConverter * getVarConverter(int size)
{
    VarConverter *v = NULL;

    switch(size) {
    case 0:     v =  &g_var0; break;
    case 8:     v =  &g_var8; break;
    case 16:    v =  &g_var16; break;
    case 32:    v =  &g_var32; break;
    case 64:    v =  &g_var64; break;
    }
    return v;
}

#define ADD_TYPE(name, size, printformat,ispointer, isEnum)                                           \
    g_varMap.insert(std::pair<std::string, VarType>(name, VarType(g_typeId++, name, &g_var##size , printformat , ispointer, isEnum)));

void TypeFactory::initBaseTypes()
{
    g_initialized = true;
    ADD_TYPE("UNKNOWN", 0, "0x%x", false, false);
    ADD_TYPE("void", 0, "0x%x", false, false);
    ADD_TYPE("char", 8, "%c", false, false);
    ADD_TYPE("int", 32, "%d", false, false);
    ADD_TYPE("float", 32, "%d", false, false);
    ADD_TYPE("short", 16, "%d", false, false);
}

int TypeFactory::initFromFile(const std::string &filename)
{
    if (!g_initialized) {
        initBaseTypes();
    }

    FILE *fp = fopen(filename.c_str(), "rt");
    if (fp == NULL) {
        perror(filename.c_str());
        return -1;
    }
    char line[1000];
    int lc = 0;
    while(fgets(line, sizeof(line), fp) != NULL) {
        lc++;
        std::string str = trim(line);
        if (str.size() == 0 || str.at(0) == '#') {
            continue;
        }
        size_t pos = 0, last;
        std::string name;
        std::string size;
        //name = getNextToken(str, pos, &last, WHITESPACE);
	name = parseTypeAndSize (str, pos, &last, &size);
        if (name.size() == 0) {
            fprintf(stderr, "Error: %d : missing type name\n", lc);
            return -2;
        }
	/*
        pos = last + 1;
        std::string size;
        size = getNextToken(str, pos, &last, WHITESPACE);
	*/
        if (size.size() == 0) {
            fprintf(stderr, "Error: %d : missing type width\n", lc);
            return -2;
        }
        pos = last + 1;
        std::string printString;
        //printString = getNextToken(str, pos, &last, WHITESPACE);
        printString = parsePrintString (str, pos, &last);
        if (printString.size() == 0) {
            fprintf(stderr, "Error: %d : missing print-string\n", lc);
            return -2;
        }

        pos = last + 1;
        std::string pointerDef;
        pointerDef = getNextToken(str, pos, &last, WHITESPACE);
        if (pointerDef.size() == 0) {
            fprintf(stderr, "Error: %d : missing ispointer definition\n", lc);
            return -2;
        }

        bool isPointer=false;
        if (std::string("true")==pointerDef)
          isPointer = true;
        else if (std::string("false")==pointerDef)
          isPointer = false;
        else
        {
          fprintf(stderr, "Error: %d : invalid isPointer definition, must be either \"true\" or \"false\"\n", lc);
          return -2;
        }

        bool isEnum = false;
        if (!strcmp ((const char*)name.c_str(), "GLenum") ||
                !strcmp ((const char*)name.c_str(), "EGLenum"))
            isEnum = true;

        VarConverter *v = getVarConverter(atoi(size.c_str()));
        if (v == NULL) {
            fprintf(stderr, "Error: %d : unknown var width: %d\n", lc, atoi(size.c_str()));
            return -1;
        }

        if (getVarTypeByName(name)->id() != 0) {
            fprintf(stderr,
                    "Warining: %d : type %s is already known, definition in line %d is taken\n",
                    lc, name.c_str(), lc);
        }
        g_varMap.insert(std::pair<std::string, VarType>(name, VarType(g_typeId++, name, v ,printString, isPointer, isEnum)));
        std::string constName = "const " + name;
        g_varMap.insert(std::pair<std::string, VarType>(constName, VarType(g_typeId++, constName, v ,printString, isPointer, isEnum))); //add a const type
    }
    g_initialized = true;
    return 0;
}


const VarType * TypeFactory::getVarTypeByName(const std::string & type)
{
    if (!g_initialized) {
        initBaseTypes();
    }
    TypeMap::iterator i = g_varMap.find(type);
    if (i == g_varMap.end()) {
        i = g_varMap.find("UNKNOWN");
    }
    return &(i->second);
}

