/* ----------------------------------------------------------------------------
 * CERTI - HLA RunTime Infrastructure
 * Copyright (C) 2002-2011  ONERA
 *
 * This program is free software ; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation ; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program ; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * ----------------------------------------------------------------------------
 * This is a modified version of tsp_check_host_and_ip.c from
 * TSP Utils Library - utilities for a generic Transport Sampling Protocol.
 *
 * Copyright (c) 2002 Yves DUFRENNE, Stephane GALLES, Eric NOULARD and Robert PAGNOT
 */

#include <iostream>
#include <algorithm>
#include <cstdlib>

#define HAVE_XML 1

#ifdef HAVE_XML
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

static int indentLevel = 0;

std::string indent() {
    std::string retval = "";
    int j = indentLevel;
    while (j>0) {
        retval = retval + "   ";
        j--;
    }
    return retval;
}

void displayCurrent(xmlNodePtr curNode) {

    xmlNodePtr cur = curNode;
    while (cur != NULL) {
    	std::cout << indent();
    	if(xmlStrcmp(cur->name,(const xmlChar*)"text")) {
         std::cout << cur->name;
    	} else {
    		std::string strcontent = (const char* )cur->content;
			strcontent.erase(remove_if(strcontent.begin(), strcontent.end(), isspace), strcontent.end());
          if (strcontent.length()>0) std::cout << " content:<" << strcontent <<">";
    	}
        std::cout << std::endl;
        if ((cur->children) != NULL) {
            indentLevel++;
            displayCurrent(cur->children);
            indentLevel--;
        }
        cur = cur->next;
    }
}

int
main(int argc, char* argv[]) {

    std::string filename = argv[1];
#ifndef HAVE_XML
    std::cerr << "CERTI has been compiled without XML support" << std::endl;
    exit(EXIT_FAILURE);
#else

    LIBXML_TEST_VERSION

    std::cerr << "CERTI compiled with XML libmlx2 version: "<< LIBXML_VERSION_STRING << std::endl;
    xmlDocPtr doc;
    xmlNodePtr current;

    doc = xmlParseFile(filename.c_str());
    if (NULL==doc) {
        std::cerr << "Unable to parse XML file:" << filename << "reason: "<< std::endl;
        xmlFreeDoc(doc);
    }
    current = xmlDocGetRootElement(doc);
    displayCurrent(current);

    xmlFreeDoc(doc);
    xmlCleanupParser();
    exit(EXIT_SUCCESS);
#endif

}
