#pragma once

#include "Header.h"
#include <fstream>
#include <cmath>
#include <cstring>
#include <string>
#include <iostream>

#define getPointerByIndex(data, byteSize, number) \
 ((uint8_t *) (data) + (byteSize) * ((number) - 1))


class FS
{
private:
    Block *fs;
    Superblock *superblock;

    uint32_t allocateDataBlock();
    uint32_t allocateINode();
    Block *getDataFromINode(uint32_t inodeNumber, uint32_t n);
    int8_t addDataBlockToObj(MetaInf *obj, uint32_t datablock);
    Block *addDataToFile(MetaInf *file);
public:
    static void create_empty_FS(std::string filename, uint32_t block_count);
    FS(std::string filename);
    void initFs();
    void saveFs(std::string filename);
    uint32_t createObj(uint8_t obj_type, std::string name);
    int8_t createLink(MetaInf *parent, uint32_t childN);
    void printFSInfo();
    MetaInf *getRootCatalog();
    Block *getPointerByDataNumber(uint32_t number);
    void printObjsInCatalog(MetaInf *catalog);
    MetaInf *findInFS(std::string name);
    void printObj(MetaInf *obj, std::string ident);
    int importDataToFile(std::string fileName, MetaInf *file);
    int exportDataFromFile(std::string fileName, MetaInf *file);
    int8_t copyFileTo(uint32_t sourceN, std::string path);
    int printDataFromFile(MetaInf *file);
    int8_t moveFileTo(std::string pathFrom, std::string pathTo);
protected:
    int8_t unLink(MetaInf *parent, uint32_t childN);
    MetaInf *findInCatalog(MetaInf *catalog, std::string name);
    INode *getPointerByINodeNumber(uint32_t number);
};