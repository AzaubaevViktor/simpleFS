#include "FS.h"
using namespace std;

uint32_t createObj(Block *fs, uint8_t obj_type, char *name);

string getFolder(string path) {
    unsigned long pos = path.find_last_of("/");
    return path.substr(0, pos + 1);
}

string getFileName(string path) {
    unsigned long pos = path.find_last_of("/");
    return path.substr(pos + 1);
}

void create_empty_FS(string filename, uint32_t block_count)
{
	ofstream fl(filename, ios_base::binary);
	string empty_block("");

	for (int i = 0; i < BS; ++i)
	{
		empty_block += '\0';
	}

	Superblock superblock;
	superblock.block_count = block_count;
	superblock.inodes_count = block_count / 10;
	uint32_t blocks_for_inodes = ceil((double)superblock.inodes_count / INODES_IN_BLOCK);
	//superblock.data_blocks_count = ;
	uint32_t inode_bit_mask_blocks = ceil((double) superblock.inodes_count / BS / 8.);
	uint32_t blocks_for_data = block_count - blocks_for_inodes - inode_bit_mask_blocks - 1;

	uint32_t data_blocks = blocks_for_data * BS * 8 / (BS * 8 + 1);
	while (data_blocks + ceil((double)data_blocks / BS / 8) < blocks_for_data)
		data_blocks += 1;

	data_blocks -= 1;
	uint32_t data_btmsk_blocks = ceil((double)data_blocks / BS / 8);

	superblock.data_blocks_count = data_blocks;

	superblock.bit_mask_inode = 2;
	superblock.first_inode = superblock.bit_mask_inode + inode_bit_mask_blocks;
	superblock.bit_mask_data = superblock.first_inode + blocks_for_inodes;
	superblock.start_data = superblock.bit_mask_data + data_btmsk_blocks;
	superblock.free_inodes = superblock.inodes_count;
	superblock.free_data_blocks = superblock.data_blocks_count;

	memcpy(superblock.fs_name, "test_fs", 8);

    for (int i = 0; i < SUPERBLOCK_TRASH_SIZE - 1; ++i) {
        superblock.trash[i] = 0;
    }
    superblock.trash[SUPERBLOCK_TRASH_SIZE - 1] = 255; // Для удобства в hex редакторе

	fl.write((char *)&superblock, BS / sizeof(char));

	for (int i = 0; i < block_count - 1; ++i)
	{
		fl << empty_block;
	}
    fl.close();
}

Block *load_FS(string filename)
{
	ifstream fs(filename, ios_base::binary);
	Block *ram_fs = new Block[1];

	fs.read((char *)ram_fs, BS / sizeof(char));
	Superblock *superblock = (Superblock *)&ram_fs[0];

    ram_fs = (Block *) realloc(ram_fs, sizeof(Block) * superblock->block_count);
    fs.read((char *) (ram_fs + 1), sizeof(Block) / sizeof(char) * (superblock->block_count - 1));
	return ram_fs;
}

void initFs(Block *fs) {
    // Проверка на то, что ФС уже проинициализирована
    createObj(fs, FS_CATALOG, string("__root_catalog"));
}

void saveFs(Block *fs, string filename) {
    ofstream fl(filename, ios_base::binary);
    fl.clear();
    Superblock *superblock = getSuperblock(fs);
    for (uint32_t blockI = 0; blockI < superblock->block_count; ++blockI) {
        fl.write((char *)(fs + blockI), BS / sizeof(char));
    }
    fl.close();
}

void clearBlock(Block *block) {
    memset(block, '\0', BS);
}

void clearINode(INode *iNode) {
    memset(iNode, '\0', INODE_SIZE);
}


int setBit(uint8_t *byte)
{
	if ((*byte & 128) == 0) {
		*byte |= 128;
		return 0;
	} elif((*byte & 64) == 0) {
		*byte |= 64;
		return 1;
	}
	elif((*byte & 32) == 0) {
		*byte |= 32;
		return 2;
	}
	elif((*byte & 16) == 0) {
		*byte |= 16;
		return 3;
	}
	elif((*byte & 8) == 0) {
		*byte |= 8;
		return 4;
	}
	elif((*byte & 4) == 0) {
		*byte |= 4;
		return 5;
	}
	elif((*byte & 2) == 0) {
		*byte |= 2;
		return 6;
	}
	elif((*byte & 1) == 0) {
		*byte |= 1;
		return 7;
	}
	return -1;
}

uint32_t allocate(Block *bitMskBlock, uint32_t bitCount)
{
    uint8_t *bitMsk = (uint8_t *) bitMskBlock;

    uint32_t i = 0;
    for (i = 0; i < ceil(bitCount / 8); ++i) {
		if (bitMsk[i] != 255) break;
	}
	int bitI = setBit(&bitMsk[i]);
	return (i * 8 + bitI + 1);
}

uint32_t allocateDataBlock(Block *fs) {
    Superblock *superblock = getSuperblock(fs);
    superblock->free_data_blocks -= 1;
	return allocate(
		fs + superblock->bit_mask_data,
		superblock->data_blocks_count
		);
}

uint32_t allocateINode(Block *fs) {
    Superblock *superblock = getSuperblock(fs);
    superblock->free_inodes -= 1;
	return allocate(
		fs + superblock->bit_mask_inode,
		superblock->inodes_count
		);

}

uint32_t createObj(Block *fs, uint8_t obj_type, string name) {
	// ������� �������� name �� ������� ���������� �������� and /
    if (string::npos != name.find('/'))
        return 0;
    Superblock *superblock = getSuperblock(fs);

	uint32_t data_block_number = allocateDataBlock(fs);
    uint32_t data_inode_number = allocateINode(fs);

	MetaInf* metainf = (MetaInf*) getPointerByDataNumber(superblock, data_block_number);
    clearBlock((Block *) metainf);
    clearINode(getPointerByINodeNumber(superblock, data_inode_number));

	metainf->length = 0;
	metainf->obj_type = obj_type;
	memcpy(metainf->name, name.c_str(), name.size() * sizeof(char));
	metainf->inode_number = data_inode_number;

	return data_block_number;
}

Block *getDataFromINode(Block *fs, uint32_t inodeNumber, uint32_t n) {
    Superblock *superblock = getSuperblock(fs);

	INode *inode = getPointerByINodeNumber(superblock, inodeNumber);
	while (n >= INODE_DATA_BLOCKS) {
		if (0 == inode->next_inode) return NULL;
		inode = getPointerByINodeNumber(superblock, inode->next_inode);
		n -= INODE_DATA_BLOCKS;
	}
    uint32_t dbn = inode->data_blocks_numbers[n];
    if (0 == dbn) return NULL;
	return getPointerByDataNumber(superblock, dbn);
}

int8_t addDataBlockToObj(Block *fs, MetaInf *obj, uint32_t datablock) {
    Superblock *superblock = getSuperblock(fs);
    INode *inode = getPointerByINodeNumber(superblock, obj->inode_number);
    uint64_t index = 0;
    while (inode->data_blocks_numbers[index] != 0) {
        index++;
        if (INODE_DATA_BLOCKS == index) {
            if (0 == inode->next_inode) inode->next_inode = allocateINode(fs);
            inode = getPointerByINodeNumber(superblock, inode->next_inode);
            index -= INODE_DATA_BLOCKS;
        }
    }
    inode->data_blocks_numbers[index] = datablock;
    return 0;
}

Block *addDataToFile(Block *fs, MetaInf *file) {
    Superblock *superblock = getSuperblock(fs);

    if (FS_FILE != file->obj_type) return NULL;
    uint32_t newDatablockN = allocateDataBlock(fs);
    addDataBlockToObj(fs, file, newDatablockN);
    return getPointerByDataNumber(superblock, newDatablockN);
}

int8_t createLink(Block *fs, MetaInf *parent, uint32_t childN) {
	if (FS_CATALOG != parent->obj_type) return -1;
	addDataBlockToObj(fs, parent, childN);
    parent->length++;
	return 0;
}

int8_t copyFileTo(Block *fs, uint32_t sourceN, string path) {
    initSuperblock;
    string folder = getFolder(path);
    MetaInf *destinationFolder = findInFS(fs, folder);
    if (!destinationFolder) return -1;

    MetaInf *source = (MetaInf *) getPointerByDataNumber(superblock, sourceN);
    if (FS_FILE != source->obj_type) return -2;

    string name = getFileName(path);
    uint32_t fdbn = createObj(fs, FS_FILE, name);

    MetaInf *destination = (MetaInf *) getPointerByDataNumber(superblock, fdbn);
    destination->length = source->length;
    createLink(fs, destinationFolder, fdbn);

    INode *sINode = getPointerByINodeNumber(superblock, source->inode_number);
    INode *dINode = getPointerByINodeNumber(superblock, destination->inode_number);

    Block *sBlock;
    Block *dBlock;

    unsigned int index = 0;
    while (sINode->data_blocks_numbers[index] != 0) {
        sBlock = getPointerByDataNumber(superblock, sINode->data_blocks_numbers[index]);
        dBlock = addDataToFile(fs, destination);
        memcpy(dBlock, sBlock, BS);
        ++index;
        if (INODE_DATA_BLOCKS == index) {
            if (!sINode->next_inode) break;
            sINode = getPointerByINodeNumber(superblock, sINode->next_inode);
            index = 0;
        }
    }
    return 0;
}

void printFSInfo(Block *fs) {
    Superblock *superblock = getSuperblock(fs);
    cout << "FS                 :" << superblock->fs_name << endl;
    cout << "Block count        :" << superblock->block_count << endl;
    cout << "INodes count       :" << superblock->inodes_count << endl;
    cout << "DataBlocks count   :" << superblock->data_blocks_count << endl;
    cout << "Free INodes        :" << superblock->free_inodes << endl;
    cout << "Free DataBlocks    :" << superblock->free_data_blocks << endl;
}

void printObj(MetaInf *obj, string ident) {
    cout << ident << obj->name;
    if (FS_CATALOG == obj->obj_type) cout << "/";
    cout << " (" << obj->length << ")[Inode:" << obj->inode_number << "]" << endl;
}

void printObjsInCatalog(Block *fs, MetaInf *catalog) {
    if (FS_CATALOG != catalog->obj_type) {
        cout << "Не каталог!" << endl;
        return;
    }
    cout << catalog->name << "/ :" << endl;
    cout << "Main INode: " << catalog->inode_number << endl;
    for (uint32_t index = 0; index < catalog->length; ++index) {
        MetaInf *obj = (MetaInf *) getDataFromINode(fs, catalog->inode_number, index);
        printObj(obj, "    ");
    }
}

MetaInf *findInCatalog (Block *fs, MetaInf *catalog, string name) {
	if (FS_CATALOG != catalog->obj_type) return NULL;
	MetaInf *object = NULL;
	for (uint32_t i = 0; i < catalog->length; ++i) {
		object = (MetaInf *) getDataFromINode(fs, catalog->inode_number, i);
		string obj_name = string((char *) object->name);
		if (obj_name == name) return object;
	}
	return NULL;
}

MetaInf *findInFS(Block *fs, string name) {
    Superblock *superblock = getSuperblock(fs);

	if ('/' != name[0]) return NULL;
    unsigned long startN = 1;
	int isCatalog = false;
	MetaInf *current = (MetaInf *) getPointerByDataNumber(superblock, ROOT_DATABLOCK_N);
    unsigned long i = startN;
	while ('\0' != name[i]) {
        i = startN;
		for (; ('/' != name[i]) && ('\0' != name[i]); ++i);
		switch (name[i])
		{
		case '/':
			current = findInCatalog(fs, current, name.substr(startN, i - startN));
			if (!current) return NULL;
			startN = i + 1;
			break;
		case '\0':
			if ('/' == name[i - 1]) {isCatalog = true;}
            else {
                current = findInCatalog(fs, current, name.substr(startN, i - startN));
                if (!current) return NULL;
            }

			break;
		default:
			break;
		}
	}
    if (isCatalog && (current->obj_type != FS_CATALOG)) return NULL;
	return current;
}

int importDataToFile(Block *fs, string fileName, MetaInf *file) {
    if (FS_FILE != file->obj_type) return -1;
    ifstream dataStream(fileName, ios_base::binary);
    Block *block;
    uint64_t size = 0;
    dataStream.seekg(0, dataStream.end);
    file->length = dataStream.tellg();
    dataStream.seekg(0, dataStream.beg);

    while (size < file->length) {
        block = getDataFromINode(fs, file->inode_number, size / BS);
        if (!block) {
            block = addDataToFile(fs, file);
        }
        dataStream.read((char *) block, BS / sizeof(char));
        size += BS;
    }
    dataStream.close();
    return 0;
}

int exportDataFromFile(Block *fs, string fileName, MetaInf *file) {
    if (FS_FILE != file->obj_type) return -1;
    ofstream dataStream(fileName, ios_base::binary);
    Block *block;
    uint64_t size = 0;
    while (size < file->length) {
        block = getDataFromINode(fs, file->inode_number, size / BS);
        dataStream.write((char *) block, min(BS, (const int &) (file->length - size)));
        size += BS;
    }
    dataStream.close();
    return 0;
}


int printDataFromFile(Block *fs, MetaInf *file) {
    uint64_t size = 0;
    Block *block;
    uint32_t index = 0;
    cout << "```` START FILE" << endl;
    while (size < file->length) {
        block = getDataFromINode(fs, file->inode_number, index);
        cout.write((const char *) block, min(BS, (const int &) (file->length - size)));
        index++;
        size += BS;
    }
    cout << "``` END FILE" << endl;
    return 0;
}