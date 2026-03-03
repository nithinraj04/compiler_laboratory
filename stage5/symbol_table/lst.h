#ifndef LST_H
#define LST_H

typedef varType varType; // Forward declaration of varType from tree.h

typedef struct gst gst; // Forward declaration of gst from gst.h
struct paramStruct;

gst* lstLookup(gst* head, char* name);
gst* lstInstall(gst* head, char* name, varType type, int ptr_level);
gst* globalLookup(gst* gstHead, gst* lstHead, char* name);
void freeLst(gst* head);
int bindParams(gst* head);

#endif