#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

void parseCTOInfo(xmlNode *ctoInfoNode);

void parseItem(xmlNode *itemNode) {
    const char *barcode = NULL;
    const char *quantity = NULL;
    const char *desc = NULL;

    for (xmlNode *curNode = itemNode->children; curNode; curNode = curNode->next) {
        if (curNode->type == XML_ELEMENT_NODE) {
            if (xmlStrcmp(curNode->name, (const xmlChar *)"Barcode") == 0) {
                barcode = (const char *)xmlNodeGetContent(curNode);
            } else if (xmlStrcmp(curNode->name, (const xmlChar *)"Quantity") == 0) {
                quantity = (const char *)xmlNodeGetContent(curNode);
            } else if (xmlStrcmp(curNode->name, (const xmlChar *)"Desc") == 0) {
                desc = (const char *)xmlNodeGetContent(curNode);
            }
        }
    }

    if (barcode) {
        printf("Barcode: %s\n", barcode);
    }
    if (quantity) {
        printf("Quantity: %s\n", quantity);
    }
    if (desc) {
        printf("Desc: %s\n", desc);
    }
    printf("\n");
}

void parseCTOInfo(xmlNode *ctoInfoNode) {
    for (xmlNode *curNode = ctoInfoNode->children; curNode; curNode = curNode->next) {
        if (curNode->type == XML_ELEMENT_NODE) {
            if (xmlStrcmp(curNode->name, (const xmlChar *)"item") == 0) {
                parseItem(curNode);
            } else if (xmlStrcmp(curNode->name, (const xmlChar *)"SubItem") == 0) {
                xmlChar *subitemBarcode = xmlGetProp(curNode, (const xmlChar *)"barcode");
                if (subitemBarcode) {
                    printf("SubItem barcode: %s\n", subitemBarcode);
                    xmlFree(subitemBarcode);
                }
                for (xmlNode *subNode = curNode->children; subNode; subNode = subNode->next) {
                    if (xmlStrcmp(subNode->name, (const xmlChar *)"CTOInfo") == 0) {
                        parseCTOInfo(subNode);
                    }
                }
            }
        }
    }
}

void parseXML(const char *filename) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    // Parse the file and get the document
    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        printf("Could not parse the XML file: %s\n", filename);
        return;
    }

    // Get the root element node
    root_element = xmlDocGetRootElement(doc);

    // Print the root element name
    printf("Root Element: %s\n", (char *)root_element->name);

    // Iterate over items in Results/CTOInfo
    for (xmlNode *resultsNode = root_element->children; resultsNode; resultsNode = resultsNode->next) {
        if (resultsNode->type == XML_ELEMENT_NODE && xmlStrcmp(resultsNode->name, (const xmlChar *)"Results") == 0) {
            for (xmlNode *ctoInfoNode = resultsNode->children; ctoInfoNode; ctoInfoNode = ctoInfoNode->next) {
                if (ctoInfoNode->type == XML_ELEMENT_NODE && xmlStrcmp(ctoInfoNode->name, (const xmlChar *)"CTOInfo") == 0) {
                    parseCTOInfo(ctoInfoNode);
                }
            }
        }
    }

    // Free the document
    xmlFreeDoc(doc);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <xml file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    parseXML(filename);
    return 0;
}