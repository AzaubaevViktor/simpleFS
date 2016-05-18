#include "Header.h"
#include "FS.h"
#include <sstream>

using namespace std;

int main() {
    FS::create_empty_FS("test.img", 20000);

    FS fs("test.img");
    fs.initFs();
    fs.printFSInfo();
    MetaInf *root_catalog = fs.getRootCatalog();
    for (int i = 0; i < 10; ++i) {
        std::ostringstream name;
        std::ostringstream catalog_name;
        catalog_name << "catalog_" << i;
        uint32_t cdbn = fs.createObj(FS_CATALOG, catalog_name.str());
        fs.createLink(root_catalog, cdbn);
        name << "file_" << i;
        uint32_t fdbn = fs.createObj(FS_FILE, name.str());
        name << "_";
        uint32_t fdbn2 = fs.createObj(FS_FILE, name.str());
        MetaInf *cat = (MetaInf *) fs.getPointerByDataNumber(cdbn);
        fs.createLink(cat, fdbn);
        fs.createLink(cat, fdbn2);
    }
    cout << "======= FS INFO ========" << endl;
    fs.printFSInfo();
    cout << "======= OBJS IN ROOT CATALOG ======" << endl;
    fs.printObjsInCatalog(root_catalog);
    cout << "======= OBJS IN 5 DATABLOCKCATALOG ======" << endl;
    fs.printObjsInCatalog((MetaInf *) fs.getPointerByDataNumber(5));
    cout << "======= FIND `/catalog_1/file_1_` file ======" << endl;
    MetaInf *finded = fs.findInFS("/catalog_1/file_1_");
    if (finded) {
        fs.printObj(finded, "finded:> ");
    } else {
        cout << "Not found!" << endl;
    }
    cout << "Load Header.h to founded file..." << endl;
    fs.importDataToFile("петросня.png", finded);

    cout << "===== EXPORT DATA TO FILE =====" << endl;
    fs.exportDataFromFile("export_test", finded);

    cout << "====== COPY FILE =====" << endl;
    fs.copyFileTo(7, "/catalog_3/copying_file");
    fs.printDataFromFile(fs.findInFS("/catalog_3/copying_file"));

    cout << "====== MOVE FILE =====" << endl;
    fs.moveFileTo("/catalog_1/file_1_", "/catalog_3/moved_file");

    cout << "====== SHOW CATALOG 3 =====" << endl;
    fs.printObjsInCatalog(fs.findInFS("/catalog_3/"));

    cout << "====== SHOW CATALOG 1 =====" << endl;
    fs.printObjsInCatalog(fs.findInFS("/catalog_1"));

    cout << "Save FS and exit" << endl;
    fs.saveFs("test2.img");
    return 0;
}