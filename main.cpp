#include "Header.h"
#include "FS.h"
#include <iostream>
#include <sstream>

using namespace std;

int main() {
    create_empty_FS("test.img", 20000);
    Block *fs = load_FS("test.img");
    initFs(fs);
    printFSInfo(fs);
    INode *rootINode = getPointerByINodeNumber((Superblock *) fs, 1);
    MetaInf *root_catalog = (MetaInf *) getPointerByDataNumber((Superblock *) fs, ROOT_DATABLOCK_N);
    for (int i = 0; i < 10; ++i) {
        std::ostringstream name;
        std::ostringstream catalog_name;
        catalog_name << "catalog_" << i;
        uint32_t cdbn = createObj(fs, FS_CATALOG, catalog_name.str());
        createLink(fs, root_catalog, cdbn);
        name << "file_" << i;
        uint32_t fdbn = createObj(fs, FS_FILE, name.str());
        name << "_";
        uint32_t fdbn2 = createObj(fs, FS_FILE, name.str());
        MetaInf *cat = (MetaInf *) getPointerByDataNumber(getSuperblock(fs), cdbn);
        createLink(fs, cat, fdbn);
        createLink(fs, cat, fdbn2);
    }
    cout << "======= FS INFO ========" << endl;
    printFSInfo(fs);
    cout << "======= OBJS IN ROOT CATALOG ======" << endl;
    printObjsInCatalog(fs, root_catalog);
    cout << "======= OBJS IN 5 DATABLOCKCATALOG ======" << endl;
    printObjsInCatalog(fs, (MetaInf *) getPointerByDataNumber(getSuperblock(fs), 5));
    cout << "======= FIND `/catalog_1/file_1_` file ======" << endl;
    MetaInf *finded = findInFS(fs, "/catalog_1/file_1_");
    if (finded) {
        printObj(finded, "finded:> ");
    } else {
        cout << "Not found!" << endl;
    }
    cout << "Load Header.h to founded file..." << endl;
    importDataToFile(fs, "петросня.png", finded);
    cout << "===== SHOW LOADED DATA =====" << endl;
//    printDataFromFile(fs, finded);
    cout << "===== EXPORT DATA TO FILE =====" << endl;
    exportDataFromFile(fs, "export_test", finded);
    cout << "====== COPY FILE =====" << endl;
    copyFileTo(fs, 7, "/catalog_3/copying_file");
    cout << "====== WRITE DATA =====" << endl;
//    printDataFromFile(fs, findInFS(fs, "/catalog_3/copying_file"));
    cout << "Save FS and exit" << endl;
    saveFs(fs, "test2.img");
    return 0;
}