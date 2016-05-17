#pragma once

#include "Header.h"
#include <fstream>
#include <cmath>
#include <cstring>
#include <string>
#include <iostream>

void create_empty_FS(std::string filename, uint32_t block_count);
Block *load_FS(std::string filename);
void initFs(Block *fs);
void saveFs(Block *fs, std::string filename);
uint32_t createObj(Block *fs, uint8_t obj_type, std::string name);
int8_t createLink(Block *fs, MetaInf *parent, uint32_t childN);
void printFSInfo(Block *fs);
void printObj(MetaInf *obj, std::string ident);
void printObjsInCatalog(Block *fs, MetaInf *catalog);
MetaInf *findInFS(Block *fs, std::string name);

int importDataToFile(Block *fs, std::string fileName, MetaInf *file);
int printDataFromFile(Block *fs, MetaInf *file);
int exportDataFromFile(Block *fs, std::string fileName, MetaInf *file);
int8_t copyFileTo(Block *fs, uint32_t sourceN, std::string path);
class FS
{

};