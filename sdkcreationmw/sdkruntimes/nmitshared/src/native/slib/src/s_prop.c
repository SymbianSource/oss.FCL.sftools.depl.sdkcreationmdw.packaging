/*
 * $Id: s_prop.c,v 1.43 2005/07/10 21:44:51 slava Exp $
 *
 * Copyright (C) 2000-2005 by Slava Monich
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 *   1.Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer. 
 *   2.Redistributions in binary form must reproduce the above copyright 
 *     notice, this list of conditions and the following disclaimer 
 *     in the documentation and/or other materials provided with the 
 *     distribution. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING 
 * IN ANY WAY OUT OF THE USE OR INABILITY TO USE THIS SOFTWARE, EVEN 
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * The views and conclusions contained in the software and documentation 
 * are those of the authors and should not be interpreted as representing 
 * any official policies, either expressed or implied.
 */

#include "s_prop.h"
#include "s_util.h"
#include "s_strbuf.h"
#include "s_itrp.h"
#include "s_mem.h"

#define DEFAULT_COMMENT_POS 32

typedef struct _PropEntry {
    QEntry e;                   /* list entry for linking them together */
    PropData data;              /* the data - key, value and such */
    int cp;                     /* zero-based comment position */
} PropEntry;

typedef struct _PropExam {      /* PROP_ExamCB context */
    PropCB cb;                  /* callback to call */
    int index;                  /* current entry index (line number) */
    Bool all;                   /* True to report full contents of the file */
    void * ctx;                 /* callback's context */
} PropExam;

typedef struct _PropMerge {     /* PROP_MergeCB context */
    StrBuf * sb;                /* string buffer to concat strings */
    Prop * dest;                /* the destination properties */
    Str prefix;                 /* the prefix to prepend */
} PropMerge;

typedef struct _PropWrite {     /* PROP_WriteCB context */
    File * out;                 /* stream we are writing to */
    StrBuf sb;                  /* temporary string buffer */
} PropWrite;

typedef struct _PropCopy {      /* PROP_CopyCB context */
    Prop * dest;                /* the destination properties */
    Bool all;                   /* copy everythign including comments */
} PropCopy;

typedef struct _PropExtract {   /* PROP_ExtractCB context */
    Prop * dest;                /* the destination properties */
    int plen;                   /* the prefix length */
    Str s1;                     /* the prefix we are searching for */
    Str s2;                     /* the string with are replacing it with */
    Bool samePrefix;            /* s1 and s2 are the same */
    StrBuf * sb;                /* the string buffer */
} PropExtract;

typedef struct _PropTag {       /* XML parsing context */
    XMLTag tag;                 /* common part */
    Bool owner;                 /* we own the Prop object */
    Prop * prop;                /* properties */
} PropTag;

/* Definition of the iterator */
STATIC IElement PROP_ItrNextKey P_((Iterator * itr));
STATIC IElement PROP_ItrNextValue P_((Iterator * itr));
STATIC IElement PROP_ItrNextData P_((Iterator * itr));
STATIC Bool PROP_ItrHasNext P_((Iterator * itr));
STATIC Bool PROP_ItrRemove P_((Iterator * itr));
STATIC void PROP_ItrFree P_((Iterator * itr));

typedef struct _PropIterator {
    Iterator itr;               /* common part */
    PropEntry * entry;          /* the current entry */
    PropEntry * next;           /* the next entry */
} PropIterator;

STATIC const Itr propKeyIterator = {
    TEXT("PropKeys"),           /* name     */
    PROP_ItrHasNext,            /* hasNext  */
    PROP_ItrNextKey,            /* next     */
    PROP_ItrRemove,             /* remove   */
    NULL,                       /* destroy  */
    PROP_ItrFree                /* free     */
};

STATIC const Itr propValueIterator = {
    TEXT("PropValues"),         /* name     */
    PROP_ItrHasNext,            /* hasNext  */
    PROP_ItrNextValue,          /* next     */
    PROP_ItrRemove,             /* remove   */
    NULL,                       /* destroy  */
    PROP_ItrFree                /* free     */
};

STATIC const Itr propDataIterator = {
    TEXT("PropData"),           /* name     */
    PROP_ItrHasNext,            /* hasNext  */
    PROP_ItrNextData,           /* next     */
    PROP_ItrRemove,             /* remove   */
    NULL,                       /* destroy  */
    PROP_ItrFree                /* free     */
};

/* XML attributes */
const Str PropRootTag            = TEXT("props");
const Str PropEntryTag           = TEXT("p");
const Str PropKeyAttr            = TEXT("k");
const Str PropValueAttr          = TEXT("v");
const Str PropCommentAttr        = TEXT("c");
STATIC const Str PropValueAttr2  = TEXT("p");

STATIC Str keyValueSeparators    = TEXT("=:");
STATIC Str commentChars          = TEXT("#!");
STATIC Str backslashChars        = TEXT(" =:#!\\\t\r\n\f");
STATIC Str backslashCharsReplace = TEXT(" =:#!\\trnf");
STATIC Str hexDigit              = TEXT("0123456789ABCDEF");
STATIC Str emptyString           = TEXT("");

/* forward definitions of static functions */
STATIC PropEntry * propSet P_((Prop * prop,Str key,Str value,Str com,int cp));

/*
 * Removes 'const' from a Prop pointer.
 */
#if DEBUG
static Prop * _PROP_Cast(const Prop * prop) {
    ASSERT(prop);
    return (Prop*)prop;
}
#else
#  define _PROP_Cast(_p) ((Prop*)(_p))
#endif /* DEBUG */

/**
 * Convert a nibble into a hex character
 */
STATIC Char PROP_ToHex(int nibble)
{
    return hexDigit[nibble & 0xF];
}

/**
 * Deallocates memory occupied by PropEntry.
 */
STATIC void PROP_DeleteEntry(PropEntry * e)
{
    if (e) {
        if (e->data.key)   MEM_Free(e->data.key);
        if (e->data.value) MEM_Free(e->data.value);
        if (e->data.c)     MEM_Free(e->data.c);
        MEM_Free(e);
    }
}

/**
 * Initialize the properties.
 */
Bool PROP_Init(Prop * prop)
{
    if (HASH_Init(&prop->map, 0, hashCompareStringKey, 
                                 stringHashProc,
                                 hashFreeNothingProc)) {
        QUEUE_Init(&prop->contents);
        return True;
    }
    return False;
}

/**
 * Dealocate all the memory used by the properties. After that, the
 * object becomes unusable.
 */
void PROP_Destroy(Prop * prop)
{
    PROP_Clear(prop);
    HASH_Destroy(&prop->map);
}

/**
 * Allocates new properties
 */
Prop * PROP_Create()
{
    Prop * prop = MEM_New(Prop);
    if (prop) {
        if (PROP_Init(prop)) {
            return prop;
        }
        MEM_Free(prop);
    }
    return NULL;
}

/**
 * Deletes the properties
 */
void PROP_Delete(Prop * prop)
{
    if (prop) {
        PROP_Destroy(prop);
        MEM_Free(prop);
    }
}

/**
 * Returns number of key/value pairs in this property set.
 */
long PROP_Size(const Prop * prop)
{
    return HASH_Size(&prop->map);
}

/**
 * Removes all entries from the properties. 
 */
void PROP_Clear(Prop * prop)
{
    QEntry * e;
    while ((e = QUEUE_RemoveHead(&prop->contents)) != NULL) {
        PropEntry * pe = QCAST(e,PropEntry,e);
        if (pe->data.key) {
            VERIFY(HASH_Remove(&prop->map,(HashKey)pe->data.key));
        }
        PROP_DeleteEntry(pe);
    }
    ASSERT(HASH_Size(&prop->map) == 0);
}

/**
 * Converts some characters into "slashed" representation.
 * Returns s if string does not contain any characters 
 * that need to be replaced
 */
STATIC Str PROP_SaveConvert(Str s, StrBuf * buf)
{
    int i;
    StrBuf * sb = NULL;

    STRBUF_Clear(buf);
    if (!s) return NULL;

    for(i=0; s[i]; i++) {
        Char c = s[i];
        Str found = StrChr(backslashChars, c);
        if (found) {
            size_t pos = found - backslashChars;
            ASSERT(pos < StrLen(backslashCharsReplace));
            if (!sb) {
                sb = buf;
                STRBUF_CopyN(sb,s,i);
            }
            STRBUF_AppendChar(sb,'\\');
            STRBUF_AppendChar(sb,backslashCharsReplace[pos]);
            continue;
        }

        if (c < 20 /*|| c > 127*/) {
            if (!sb) {
                sb = buf;
                STRBUF_CopyN(sb,s,i);
            }
            STRBUF_Append(sb, TEXT("\\u"));
            STRBUF_AppendChar(sb,PROP_ToHex((c >> 12) & 0xF));
            STRBUF_AppendChar(sb,PROP_ToHex((c >> 8)  & 0xF));
            STRBUF_AppendChar(sb,PROP_ToHex((c >> 4)  & 0xF));
            STRBUF_AppendChar(sb,PROP_ToHex((c >> 0)  & 0xF));
            continue;
        }

        if (sb) STRBUF_AppendChar(sb,c);
    }

    return (sb ? sb->s : s);
}

/**
 * Converts saved characters from "slashed" representation into
 * normal one. Returns s if string does not contain any characters 
 * that need to be replaced. Otherwise returns pointer to string
 * buffers containing converted string.
 */
STATIC Str PROP_LoadConvert(Str s, StrBuf * buf)
{
    int i;
    StrBuf * sb = NULL;

    STRBUF_Clear(buf);
    if (!s) return NULL;

    for (i=0; s[i];) {
        Char c = s[i++];
        if ((c == '\\') && s[i]) {
            if (!sb) {
                sb = buf;
                STRBUF_CopyN(sb,s,i-1);
            }
            c = s[i++];
            if (c == 'u') {
                
                // read the xxxx
                int j, value=0;
                for (j=0; j<4; j++) {
                    c = s[i++];
                    switch (c) {
                    case '0': case '1': 
                    case '2': case '3': 
                    case '4': case '5': 
                    case '6': case '7': 
                    case '8': case '9':
                        value = (value << 4) + c - '0';
                        break;
                    case 'a': case 'b': case 'c':
                    case 'd': case 'e': case 'f':
                        value = (value << 4) + 10 + c - 'a';
                        break;
                    case 'A': case 'B': case 'C':
                    case 'D': case 'E': case 'F':
                        value = (value << 4) + 10 + c - 'A';
                        break;
                    }
                }
                STRBUF_AppendChar(sb,(Char)value);
            } else {
                Str bc = StrChr(backslashCharsReplace, c);
                ASSERT(bc);
                if (bc) {
                    int pos = bc - backslashCharsReplace;
                    STRBUF_AppendChar(sb,backslashChars[pos]);
                }
            }
        } else {
            if (sb) STRBUF_AppendChar(sb,c);
        }
    }
    return (sb ? sb->s : s);
}

/**
 * Trim trailing spaces unless they are "escaped" by backslash
 */
STATIC void PROP_LoadTrimTail(StrBuf * sb)
{
    while (sb->len && IsSpace(sb->s[sb->len-1]) && 
          (sb->len<2 || sb->s[sb->len-2] != '\\')) {
        STRBUF_SetLength(sb, sb->len-1);
    }
}

/**
 * Adds comment entry to the properties
 */
STATIC Bool PROP_AddCommentEntry(Prop * prop, Str c, int cp)
{
    PropEntry * pe = MEM_New(PropEntry);
    if (pe) {
        memset(pe, 0, sizeof(*pe));
        pe->cp = cp;
        if (c) pe->data.c = STRING_Dup(c);
        if (pe->data.c || !c) {
            pe->data.index = prop->contents.size;
            QUEUE_InsertTail(&prop->contents, &pe->e);
            return True;
        }
        MEM_Free(pe);
    }
    return False;
}

/**
 * Adds empty entry to the properties
 */
STATIC Bool PROP_AddEmptyEntry(Prop * prop)
{
    PropEntry * pe = MEM_New(PropEntry);
    if (pe) {
        memset(pe, 0, sizeof(*pe));
        pe->data.index = prop->contents.size;
        QUEUE_InsertTail(&prop->contents, &pe->e);
        return True;
    }
    return False;
}

/**
 * Parses input line.
 */
STATIC Bool PROP_Parse(Prop * prop, const StrBuf * line)
{
    
    Bool success = False;
    Str com;
    StrBuf kb;    
    StrBuf kb1;
    size_t pos,i,i2;
    size_t len = STRBUF_Length(line);

    /* empty string? */    
    if (len == 0) {
        return PROP_AddEmptyEntry(prop);
    }

    /* skip whitespace */
    i = 0;
    while ((i<len) && IsSpace(STRBUF_CharAt(line,i))) i++;

    /* still anything left? */
    if (i == len) {
        return PROP_AddEmptyEntry(prop);
    }

    pos = i;
    com = StrChr(commentChars,STRBUF_CharAt(line,pos));

    /* is that a comment? */
    if (com) {
        return PROP_AddCommentEntry(prop, line->s+i+1, pos);
    }

    /* find separator */
    for (i2=i; i2<len; i2++, pos++) {
        Char c = STRBUF_CharAt(line, i2);
        if (c == '\\') {
            pos++;
            i2++;
            continue;
        } else if (StrChr(keyValueSeparators,c)) {
            pos++;
            break;
        }
    }
        
    /* extract the key */
    STRBUF_Init(&kb);
    STRBUF_Init(&kb1);
    if (STRBUF_CopyN(&kb, STRBUF_GetString(line)+i, i2-i)) {
        Str key;

        /* trim trailing spaces unless they are "escaped" by backslash */
        PROP_LoadTrimTail(&kb);
        key = PROP_LoadConvert(kb.s, &kb1);
        if (key) {

            /* have we actually found the separator? */
            if (i2 < len) {

                /* skip more spaces */
                size_t j = i2+1;
                while ((j<len) && IsSpace(STRBUF_CharAt(line,j))) {
                    j++;
                    pos++;
                }

                if (j<len) {

                    /* extract the key */
                    StrBuf vb;
                    StrBuf vb1;
                    STRBUF_Init(&vb);
                    STRBUF_Init(&vb1);
                                      
                    if (STRBUF_CopyN(&vb,line->s+j,len-j)) {
                        
                        size_t k;
                        Str value;

                        StrBuf cb;
                        STRBUF_Init(&cb);

                        /* find non-escaped comment char */
                        for (k=j; k<len; k++, pos++) {
                            Char c = STRBUF_CharAt(line, k);
                            if (c == '\\') {
                                k++;
                                pos++;
                                continue;
                            } else if (StrChr(commentChars,c)) {
                                break;
                            }
                        }

                        /* found end-of-line comment? */
                        if (k < len) {
                            VERIFY(STRBUF_CopyN(&cb,line->s+k+1,len-k-1));
                            STRBUF_SetLength(&vb, k-j);
                        }
                        
                        PROP_LoadTrimTail(&vb);
                        value = PROP_LoadConvert(vb.s, &vb1);
                        success = BoolValue(propSet(prop,key,value,cb.s,pos));
                        
                        STRBUF_Destroy(&cb);
                    }

                    STRBUF_Destroy(&vb);
                    STRBUF_Destroy(&vb1);
                } else {
                    success = PROP_Set(prop, key, emptyString);
                }
            } else if (*key) {
                PROP_Set(prop, key, emptyString);
            }
        }
    }
    STRBUF_Destroy(&kb);
    STRBUF_Destroy(&kb1);
    return success;
}

/**
 * Reads properties from the open file.
 */
Bool PROP_Read(Prop * prop, File * in)
{

    Bool success = True;
    StrBuf line;
    STRBUF_Init(&line);
    PROP_Clear(prop);

    while (FILE_ReadLine(in,&line)) {

        /* advance beyond whitespace on new line */
        size_t i = 0, len = STRBUF_Length(&line);
        while ((i<len) && IsSpace(line.s[i])) i++;
        if (i>=len) {
            if (!PROP_AddEmptyEntry(prop)) {
                success = False;
                break;
            }
        } else {

            /* comments? */
            Str com = StrChr(commentChars,line.s[i]);
            if (com) {
                if (PROP_AddCommentEntry(prop, line.s+i+1, i)) {
                    continue;
                } else {
                    success = False;
                    break;
                }         
            }

            /* parse the string */
            if (!PROP_Parse(prop, &line)) {
                success = False;
                break;
            }
        }
    }

    STRBUF_Destroy(&line);
    return success;
}

/**
 * Callback that writes properties into the file
 */
STATIC Bool PROP_WriteCB(QEntry * e, void * ctx)
{
    PropWrite * context = (PropWrite*)ctx;
    StrBuf * sb = &context->sb;
    File * out = context->out;
    int pos = 0;

    PropEntry * pe = QCAST(e,PropEntry,e);
    if (pe->data.key) {

        Str s;
        Char sep[4];

        s = PROP_SaveConvert(pe->data.key, sb);
        if (!FILE_Puts(out, s))  {
            return False;
        }
        pos += StrLen(s);

        sep[0] = sep[2] = ' ';
        sep[1] = keyValueSeparators[0];
        sep[3] = 0;
        if (!FILE_Puts(out, sep)) {
            return False;
        }
        pos += 3;

        if (pe->data.value) {
            s = PROP_SaveConvert(pe->data.value, sb);
            if (!FILE_Puts(out, s))  {
                return False;
            }
            pos += StrLen(s);

            if (!FILE_Putc(out, ' ')) {
                return False;
            }
            pos++;
        }
    } 

    if (pe->cp >= 0 && pe->data.c) {
        while (pos < pe->cp) {
            if (!FILE_Putc(out, ' ')) {
                return False;
            }
            pos++;
        }
        
        if (!FILE_Putc(out, commentChars[0])) {
            return False;
        }

        if (!IsSpace(pe->data.c[0]) && !FILE_Putc(out, ' ')) {
            return False;
        }
        
        if (!FILE_Puts(out, pe->data.c)) {
            return False;
        }
        
    }
    
    /* end of line */
    if (!FILE_Putc(out, '\n')) {
        return False;
    }

    return True;
}

/**
 * Writes properties into the open file.
 */
Bool PROP_Write(const Prop * prop, File * out)
{
    Bool success;
    Queue * contents = &(_PROP_Cast(prop)->contents);

    PropWrite context;
    context.out = out;
    STRBUF_Init(&context.sb);

    success = QUEUE_Examine(contents, PROP_WriteCB, &context);

    STRBUF_Destroy(&context.sb);
    if (success) {
        return True;
    }

    TRACE("error writing properties!\n");
    return False;
}

/**
 * Writes one entry in XML format to the output stream
 */
STATIC Bool PROP_WriteEntryCB(const PropData * entry, void * ctx)
{
    File * out = (File*)ctx;
    if (XML_StartTag(out, PropEntryTag)) {
        if (entry->key) {
            if (!XML_WriteAttr(out, PropKeyAttr, entry->key) ||
                (entry->value &&
                !XML_WriteAttr(out, PropValueAttr, entry->value))) {
                return False;
            }
        }
        if (entry->c &&
            !XML_WriteAttr(out, PropCommentAttr, entry->c)) {
            return False;
        }
        return XML_EndTag(out, True);
    }
    return False;
}

/**
 * Writes properties (only the data entries, no root tag) in XML format
 * to the output stream. The output look like this:
 *
 *   <p k="foo" v="bar"/>
 *   <p k="key" v="value="/>
 */
Bool PROP_WriteDataAsXML(const Prop * prop, File * out)
{
    return PROP_ExamineAll(prop, PROP_WriteEntryCB, out);
}

/**
 * Writes properties in XML format to the output stream. The output look 
 * like this:
 *
 *   <props>
 *     <p k="foo" v="bar"/>
 *     <p k="key" v="value="/>
 *   </props>
 *
 * Where <props> tag can be replace with the specified tag (the second
 * parameter). If the second parameter is NULL, the default <props> tag
 * is used.
 */
Bool PROP_WriteXML(const Prop * prop, Str tag, File * out)
{
    if (!tag || !tag[0]) tag = PropRootTag;
    return BoolValue(XML_OpenTag(out, tag, True) &&
                     PROP_WriteDataAsXML(prop, out) &&
                     XML_CloseTag(out, tag, True));
}

/**
 * Handles the children of the root (normally, <props>) tag
 */
STATIC XMLTag * PROP_LoadCB(XMLTag * p, Str name, const XMLAttr * a)
{
    if (StrCmp(name, PropEntryTag) == 0) {
        static XMLTag PropEntryXMLTag = {NULL, NULL, XML_DontFreeTagCB};
        PropTag * propTag = CAST(p,PropTag,tag);
        Str key = XML_AttrValue(a, PropKeyAttr);
        Str value = XML_AttrValue(a, PropValueAttr);
        Str comment = XML_AttrValue(a, PropCommentAttr);
        if (!value) value = XML_AttrValue(a, PropValueAttr2);
        if (key && value) {
            if (propSet(propTag->prop, key, value, comment, -1)) {
                return &PropEntryXMLTag;
            }
        } else if (comment) {
            if (PROP_Comment(propTag->prop, comment)) {
                return &PropEntryXMLTag;
            }
        }
    } else {
        Warning(TEXT("PROP: Tag <%s> is not a valid property tag\n"),name);
    }
    return NULL;
}

/**
 * Destructor for PropTag allocated by PROP_RootCB
 */
STATIC Bool PROP_EndRootTag(XMLTag * tag, XMLTag * parent)
{
    PropTag * root;
    UNREF(parent);
    
    root = CAST(tag,PropTag,tag);
    if (root->owner) PROP_Delete(root->prop);
    MEM_Free(root);
    return True;
}

/**
 * Returns the properties loaded from the XML stream. Deallocates the
 * tag context
 */
Prop * PROP_FromTag(XMLTag * tag)
{
    PropTag * root = CAST(tag,PropTag,tag);
    Prop * prop = root->prop; 
    MEM_Free(root);
    return prop;
}

/**
 * Top level callback for parsing XML stream. The context parameter may 
 * point to an existing Prop object. If context is NULL, a new Prop 
 * structure is created.
 */
XMLTag * PROP_RootCB(void * context, Str tag, const XMLAttr * a)
{
    PropTag * root;
    UNREF(tag);
    UNREF(a);

    root = MEM_New(PropTag);
    if (root) {
        memset(root, 0, sizeof(*root));
        if (context) {
            root->prop = (Prop*)context;
        } else {
            root->prop = PROP_Create();
            if (root->prop) {
                root->owner = True;
            }
        }
        if (root->prop) {
            root->tag.handleTag = PROP_LoadCB;
            root->tag.endTag = PROP_EndRootTag;
            return &root->tag;
        }
        MEM_Free(root);
    }
    return NULL;
}

#ifndef _NT_KERNEL
/**
 * Read properties from the file specified by name.
 */
Bool PROP_Load(Prop * prop, Str fname, IODesc io)
{
    Bool success = False;
    File * in = FILE_Open(fname, READ_TEXT_MODE, io);
    if (in) {
        success = PROP_Read(prop, in);
        FILE_Close(in);
    } else {
        PROP_Clear(prop);
        TRACE1("cannot open file %s\n",fname);
    }
    return success;
}

/**
 * Properties save callback
 */
STATIC Bool PROP_FileSaveCB(File * out, Str fname, void * ctx)
{
    Verbose(TEXT("Saving properties into %s\n"),fname);
    return PROP_Write((Prop*)ctx, out);
}

/**
 * Save properties into the file. This function does it carefully, saving 
 * data into temporary file X, then renames X into the specified file.
 * This way the original contents never gets lost.
 */
Bool PROP_Save(const Prop * prop, Str fname, IODesc io)
{
    return FILE_Save(fname, PROP_FileSaveCB, _PROP_Cast(prop), io);
}

/**
 * Properties save callback (XML format)
 */
STATIC Bool PROP_FileSaveXML(File * out, Str fname, void * ctx)
{
    Bool ok = False;
    File * f = out;
    File * wrap;

    UNREF(fname);

    /* for better formatting */
    wrap = FILE_Wrap(out, 2, 78);
    if (wrap) f = wrap;
    
    /* write XML prolog */
    if (FILE_Puts(f,TEXT(XML_PROLOG)) &&
        FILE_Puts(f,TEXT("\n")) &&
        FILE_Puts(f,TEXT("<!-- Properties saved on ")) &&
        FILE_Puts(f,TIME_ToString(TIME_Now())) &&
        FILE_Puts(f,TEXT(" -->\n"))) {

        /* write properties */
        ok = PROP_WriteXML((Prop*)ctx, NULL, f);
    }

    /* deallocate wrapper */
    if (wrap) {
        FILE_Detach(wrap);
        FILE_Close(wrap);
    }
    return ok;
}

/**
 * Saves properties in XML format into the file
 */
Bool PROP_SaveXML(const Prop * prop, Str fname, IODesc io)
{
    return FILE_Save(fname, PROP_FileSaveXML, _PROP_Cast(prop), io);
}

#endif /* _NT_KERNEL */

/**
 * Returns string property.
 */
Str PROP_Get(const Prop * prop, Str key)
{
    PropEntry * e = (PropEntry*)HASH_Get(&prop->map, (HashKeyC)key);
    return (e ? e->data.value : NULL);
}

/**
 * Returns integer property. Returns True if property exists and parseable
 * as an integer number. The parsed value is stored in the location pointed
 * to by parameter n.
 */
Bool PROP_GetInt(const Prop * prop, Str key, int * n)
{
    return PARSE_Int(PROP_Get(prop, key), n, 0);    
}

/**
 * Returns integer property. Returns True if property exists and parseable
 * as an integer number. The parsed value is stored in the location pointed
 * to by parameter n.
 */
Bool PROP_GetUInt(const Prop * prop, Str key, unsigned int * n)
{
    return PARSE_UInt(PROP_Get(prop, key), n, 0);    
}

/**
 * Returns long property. Returns True if property exists and parseable
 * as a long number. The parsed value is stored in the location pointed
 * to by parameter n.
 */
Bool PROP_GetLong(const Prop * prop, Str key, long * n)
{
    return PARSE_Long(PROP_Get(prop, key), n, 0);    
}

/**
 * Returns long property. Returns True if property exists and parseable
 * as a long number. The parsed value is stored in the location pointed
 * to by parameter n.
 */
Bool PROP_GetULong(const Prop * prop, Str key, unsigned long * n)
{
    return PARSE_ULong(PROP_Get(prop, key), n, 0);    
}

/**
 * Returns boolean property. Returns True if property exists and parseable
 * as boolean value. The parsed value is stored in the location pointed
 * to by parameter b.
 */
Bool PROP_GetBool(const Prop * prop, Str key, Bool * b)
{
    return PARSE_Bool(PROP_Get(prop, key), b);    
}

/**
 * Returns boolean property as an int. Returns True if property exists and 
 * parseable as boolean value. The parsed value is stored in the location 
 * pointed to by parameter b.
 */
Bool PROP_GetIntBool(const Prop * prop, Str key, int * b)
{
    Bool tmp;
    if (PARSE_Bool(PROP_Get(prop, key), &tmp)) {
        if (b) *b = tmp;
        return True;
    }
    return False;
}

/**
 * Returns double property. Returns True if property exists and parseable
 * as an double number. The parsed value is stored in the location pointed
 * to by parameter d.
 */
#ifndef __KERNEL__
Bool PROP_GetDouble(const Prop * prop, Str key, double * d)
{
    return PARSE_Double(PROP_Get(prop, key), d);    
}
#endif /* __KERNEL__ */

/**
 * Returns comment for specified entry.
 */
Str PROP_GetComment(const Prop * prop, Str key)
{
    PropEntry * e = (PropEntry*)HASH_Get(&prop->map, (HashKeyC)key);
    return (e ? e->data.c : NULL);
}

/**
 * get/set 64 bit numbers
 */
#ifndef __LONG_64__
Bool PROP_GetLong64(const Prop * prop, Str key, I64s * n)
{
    return PARSE_Long64(PROP_Get(prop, key), n, 0);    
}

Bool PROP_SetLong64(Prop * prop, Str key, I64s n)
{
    Char buf[42];
    Char * s = STRING_Format(buf,sizeof(buf),TEXT(LONG_LONG_FORMAT),n);
    Bool success = PROP_Set(prop, key, s);
    if (s != buf) MEM_Free(s);
    return success;
}
#endif /* !__LONG_64__ */

/**
 * Internal property setter. Returns NULL if memory allocation fails.
 */
STATIC PropEntry * propSet(Prop * prop, Str key, Str value, Str com, int cp)
{
    if (key && value) {
        PropEntry * pe = (PropEntry*)HASH_Get(&prop->map, (HashKeyC)key);
        if (pe) {
            Char * val;
            ASSERT(StrCmp(key, pe->data.key) == 0);
            if (StrCmp(value, pe->data.value) == 0) {
                /* 
                 * Properties already contain the exact same value. 
                 * Don't touch anything, just return existing entry.
                 */
                return pe;
            }

            /*
             * Otherwise we have to replace the value.
             */
            val = STRING_Dup(value);
            if (val) {
                MEM_Free(pe->data.value);
                pe->data.value = val;
                return pe;
            }

        } else {
            pe = MEM_New(PropEntry);
            if (pe) {
                pe->data.key = STRING_Dup(key);
                if (pe->data.key) {
                    pe->data.value = STRING_Dup(value);
                    if (pe->data.value) {
                        HashKey hashKey = (HashKey)pe->data.key;
                        HashValue hashValue = (HashValue)pe;
                        pe->data.c = STRING_Dup(com);
                        pe->cp = cp;
                        if (HASH_Put(&prop->map, hashKey, hashValue)) {
                            pe->data.index = prop->contents.size;
                            QUEUE_InsertTail(&prop->contents, &pe->e);
                            return pe;
                        }
                        MEM_Free(pe->data.value);
                    }
                    MEM_Free(pe->data.key);
                }
                MEM_Free(pe);
            }
        }
    }

    return NULL;
}

/**
 * Set property. Returns False if memory allocation fails.
 */
Bool PROP_Set(Prop * prop, Str key, Str value)
{
    return BoolValue(propSet(prop, key, value, NULL, -1) != NULL);
}

/**
 * Sets integer property.
 */
Bool PROP_SetInt(Prop * prop, Str key, int n)
{
    Char buf[32];
    Char * s = STRING_Format(buf,COUNT(buf),TEXT("%d"),n);
    Bool success = PROP_Set(prop, key, s);
    if (s != buf) MEM_Free(s);
    return success;
}

/**
 * Sets integer property.
 */
Bool PROP_SetUInt(Prop * prop, Str key, unsigned int n)
{
    Char buf[32];
    Char * s = STRING_Format(buf,COUNT(buf),TEXT("%u"),n);
    Bool success = PROP_Set(prop, key, s);
    if (s != buf) MEM_Free(s);
    return success;
}

/**
 * Sets long property.
 */
Bool PROP_SetLong(Prop * prop, Str key, long n)
{
    Char buf[32];
    Char * s = STRING_Format(buf,COUNT(buf),TEXT("%ld"),n);
    Bool success = PROP_Set(prop, key, s);
    if (s != buf) MEM_Free(s);
    return success;
}

/**
 * Sets long property.
 */
Bool PROP_SetULong(Prop * prop, Str key, unsigned long n)
{
    Char buf[32];
    Char * s = STRING_Format(buf,COUNT(buf),TEXT("%lu"),n);
    Bool success = PROP_Set(prop, key, s);
    if (s != buf) MEM_Free(s);
    return success;
}

/**
 * Sets boolean property.
 */
Bool PROP_SetBool(Prop * prop, Str key, Bool b)
{
    return PROP_Set(prop, key, b ? TRUE_STRING : FALSE_STRING);
}

/**
 * Sets double property.
 */
#ifndef __KERNEL__
Bool PROP_SetDouble(Prop * prop, Str key, double d)
{
    Bool ok = True;
    StrBuf64 buf;
    STRBUF_InitBufXXX(&buf);
    if (STRING_FormatDouble(&buf.sb, d)) {
        ok = PROP_Set(prop, key, STRBUF_Text(&buf.sb));
    }    
    STRBUF_Destroy(&buf.sb);
    return ok;
}
#endif /* __KERNEL__ */

/**
 * Sets comment for the specified key.
 */
Bool PROP_SetComment(Prop * prop, Str key, Str com)
{
    Bool created = False;
    HashKey hashKey = (HashKey)key;
    PropEntry * e = (PropEntry*)HASH_Get(&prop->map, hashKey);

    if (!e) {
        if (!com) {
            return True;
        }

        created = True;
        VERIFY(PROP_Set(prop, key, emptyString));
        e = (PropEntry*)HASH_Get(&prop->map, hashKey);
        ASSERT(e);
    }

    if (e) {
        Char * c = NULL;
        if (!e->data.c && !com) {
            return True;
        } else if (e->data.c && com && !StrCmp(e->data.c, com)) {
            return True;
        }

        if (com) {
            c = STRING_Dup(com);
            if (!c) {

                /*
                 * If we have created the entry with a single purpose of
                 * storing the comment, and then we failed to allocate a
                 * string, get rid of the entry. If it existed before,
                 * don't touch it.
                 */

                if (created) {
                    PROP_Remove(prop, key);
                }

                return False;
            }
        }

        if (e->data.c) {
            MEM_Free(e->data.c);
        }

        e->data.c = c;
        if (e->cp < 0) e->cp = DEFAULT_COMMENT_POS;
        return True;
    }

    return False;
}

/**
 * Adds a comment that has no key.
 */
Bool PROP_Comment(Prop * prop, Str com)
{
    return PROP_AddCommentEntry(prop, com, 0);
}

/**
 * Removes the existing entry from the properties
 */
STATIC void PROP_RemoveEntry(PropEntry * pe)
{
    /*
     * note that as a result of removal of a key/value pair, the entry
     * indices may get out of sync. However, there's no way user can 
     * access those except via PROP_Examine which will fix the indices
     */
    Prop * prop = CAST_QUEUE(pe->e.queue,Prop,contents);
    ASSERT(HASH_Get(&prop->map, pe->data.key) == pe);
    VERIFY(HASH_Remove(&prop->map, pe->data.key));
    QUEUE_RemoveEntry(&pe->e);
    PROP_DeleteEntry(pe);
}

/**
 * Remove the key/value pair from the properties. Returns True if entry
 * has been removed, False if nothing existed in the table for this key.
 */
Bool PROP_Remove(Prop * prop, Str key)
{
    HashKey hashKey = (HashKey)key;
    PropEntry * pe = (PropEntry*)HASH_Get(&prop->map, hashKey);
    if (pe) {
        PROP_RemoveEntry(pe);
        return True;
    }
    return False;
}

/**
 * Moves the contents of one Prop object into another. The contents of the
 * destination object gets destroyed. This function never fails because it
 * does not allocate new memory, it just moves stuff from one place to 
 * another
 */
void PROP_Move(Prop * dest, Prop * src)
{
    HashTable tmp;
    PROP_Clear(dest);

    /* swap the hash tables */
    tmp = dest->map;
    dest->map = src->map;
    src->map = tmp;

    /* move the list entries */
    QUEUE_Move(&dest->contents, &src->contents);
}

/**
 * Callback for PROP_Copy
 */
STATIC Bool PROP_CopyCB(const PropData * entry, void * ctx)
{
    Bool ok = True;
    PropCopy * copy = (PropCopy*)ctx;
    if (entry->key) {
        ok = PROP_Set(copy->dest, entry->key, entry->value);
        if (copy->all && ok && entry->c) {
            ok = PROP_SetComment(copy->dest, entry->key, entry->c);
        }
    } else {
        ASSERT(copy->all);
        ok = PROP_Comment(copy->dest, entry->c);
    }
    return ok;
}

/**
 * Copies all properties from the one Prop object to another.
 * If memory allocation fails, this function return False and
 * the destination object may be damaged (some of the source
 * properties may have been copied)
 *
 * This function does not copy empty lines and comments
 */
Bool PROP_Copy(Prop * dest, const Prop * src)
{
    PropCopy copy;
    copy.dest = dest;
    copy.all = False;
    return PROP_Examine(src, PROP_CopyCB, &copy);
}

/**
 * Copies all properties from the one Prop object to another,
 * including the entries that contain no data (i.e. empty or 
 * comment lines). If memory allocation fails, this function 
 * return False and the destination object may be damaged (some 
 * of the source properties may have been copied)
 */
Bool PROP_CopyAll(Prop * dest, const Prop * src)
{
    PropCopy copy;
    copy.dest = dest;
    copy.all = True;
    return PROP_ExamineAll(src, PROP_CopyCB, &copy);
}

/**
 * Callback for PROP_Extract
 */
STATIC Bool PROP_ExtractCB(const PropData * entry, void * ctx)
{
    PropExtract * ext = (PropExtract*)ctx;
    ASSERT(entry->key);
    if (!ext->s1 || STRING_StartsWith(entry->key, ext->s1)) {
        Str key = entry->key;
        if (!ext->samePrefix) {
            if (ext->s2) {
                STRBUF_Clear(ext->sb);
                if (!STRBUF_Copy(ext->sb, ext->s2) ||
                    !STRBUF_Append(ext->sb, entry->key + ext->plen)) {
                    return False;
                }
                key = STRBUF_GetString(ext->sb);
            } else {
                key = entry->key + ext->plen;
            }
        }
        return PROP_Set(ext->dest, key, entry->value);
    }
    return True;
}

/**
 * Copies the those entries from the source properties that have names
 * starting with s1 prefix. The s1 prefix is replaced with s2 prefix
 * prior to adding property to the destination set. This function does
 * not copy comments, only the data.
 */
Bool PROP_Extract(Prop * dest, Str s1, Str s2, const Prop * src)
{
    Bool ok;
    StrBuf32 buf;
    PropExtract ext;

    STRBUF_InitBufXXX(&buf);
    ext.dest = dest;
    ext.plen = (s1 ? StrLen(s1) : 0);
    ext.s1 = s1;
    ext.s2 = s2;
    ext.samePrefix = BoolValue(s1 == s2 || (s1 && s2 && StrCmp(s1,s2) == 0));
    ext.sb = &buf.sb;

    ok = PROP_Examine(src, PROP_ExtractCB, &ext);
    STRBUF_Destroy(&buf.sb);
    return ok;
}

/**
 * Callback for PROP_Merge
 */
STATIC Bool PROP_MergeCB(const PropData * entry, void * ctx)
{
    PropMerge * merge = (PropMerge*)ctx;
    if (entry->key) {
        Str destKey = entry->key;
        if (merge->prefix) {
            STRBUF_Clear(merge->sb);
            STRBUF_Copy(merge->sb, merge->prefix);
            STRBUF_Append(merge->sb, entry->key);
            destKey = STRBUF_Text(merge->sb);
        }
        return PROP_Set(merge->dest, destKey, entry->value);
    }
    return True;
}

/**
 * Merges the source properties with the destination, prepending the
 * specified prefix. This function only copies the data, ignoring the
 * comments in the source property set
 */
void PROP_Merge(Prop * dest, Str prefix, const Prop * src)
{
    if (PROP_Size(src) > 0) {
        PropMerge merge;
        StrBuf32 buf;
        STRBUF_InitBufXXX(&buf);
        merge.sb = &buf.sb;
        merge.dest = dest;
        merge.prefix = prefix;
        PROP_Examine(src, PROP_MergeCB, &merge);
        STRBUF_Destroy(&buf.sb);
    }
}

/**
 * Internal PROP_Examine callback
 * Returns True to continue, False to stop examining the queue.
 */
STATIC Bool PROP_ExamCB(QEntry * e, void * ctx)
{
    Bool retval = True;
    PropExam * exam = (PropExam*)ctx;    
    PropEntry * pe = CAST_ENTRY(e,PropEntry,e);
    pe->data.index = exam->index;
    if (exam->all || pe->data.key) {
        retval = (*exam->cb)(&pe->data, exam->ctx);
    }
    exam->index++;
    return retval;
}

/**
 * Invoke user callback on each key/value stored in properties
 */
STATIC Bool PROP_Examine2(const Prop * prop, PropCB cb, void * ctx, Bool all)
{
    PropExam exam;
    exam.cb = cb;
    exam.ctx = ctx;
    exam.all = all;
    exam.index = 0;
    return QUEUE_Examine(QUEUE_Cast(&prop->contents), PROP_ExamCB, &exam);
}

/**
 * Invoke user callback on each key/value stored in the properties.
 */
Bool PROP_Examine(const Prop * prop, PropCB cb, void * ctx)
{
    return PROP_Examine2(prop, cb, ctx, False);
}

/**
 * Invoke user callback on each entry stored in the properties, including
 * those containing no data (i.e. empty or comment entries)
 */
Bool PROP_ExamineAll(const Prop * prop, PropCB cb, void * ctx)
{
    return PROP_Examine2(prop, cb, ctx, True);
}

/* 
 * Initializes PropIterator
 */
STATIC Iterator * PROP_ItrCreate(Prop * prop, const Itr * type)
{
    if (PROP_Size(prop) == 0) {
        return ITR_Empty();
    } else {
        PropIterator * pi = MEM_New(PropIterator);
        if (pi) {
            QEntry * e = QUEUE_First(&prop->contents);
            ITR_Init(&pi->itr, type);
            pi->entry = NULL;
            pi->next = NULL;
            while (e) {
                PropEntry * pe = QCAST(e,PropEntry,e);
                if (pe->data.key) {
                    pi->next = pe;
                    break;
                }
                e = QUEUE_Next(e);
            }
            return &pi->itr;
        } else {
            return NULL;
        }
    }
}

/* 
 * Creates an iterator that returns property keys
 */
Iterator * PROP_Keys(Prop * prop)
{
    return PROP_ItrCreate(prop, &propKeyIterator);
}

/* 
 * Creates an iterator that returns property values 
 */
Iterator * PROP_Values(Prop * prop)
{
    return PROP_ItrCreate(prop, &propValueIterator);
}

/* 
 * Creates an iterator that returns pointers to PropData
 */
Iterator * PROP_Data(Prop * prop)
{
    return PROP_ItrCreate(prop, &propDataIterator);
}

/*==========================================================================*
 *              I T E R A T O R S
 *==========================================================================*/

/**
 * Advances the hash iterator to the next position
 */
STATIC void PROP_ItrAdvance(PropIterator * pi)
{
    pi->entry = pi->next;
    if (pi->entry) {
        QEntry * e = &pi->entry->e;
        while ((e = QUEUE_Next(e)) != NULL) {
            PropEntry * pe = QCAST(e,PropEntry,e);
            if (pe->data.key) {
                pi->next = pe;
                return;
            }
        }
        pi->next = NULL;
    }
}

STATIC IElement PROP_ItrNextKey(Iterator * itr)
{
    PropIterator * pi = CAST(itr,PropIterator,itr);
    Char * key = pi->next->data.key;
    PROP_ItrAdvance(pi);
    return key;
}

STATIC IElement PROP_ItrNextValue(Iterator * itr)
{
    PropIterator * pi = CAST(itr,PropIterator,itr);
    Char * value = pi->next->data.value;
    PROP_ItrAdvance(pi);
    return value;
}

STATIC IElement PROP_ItrNextData(Iterator * itr)
{
    PropIterator * pi = CAST(itr,PropIterator,itr);
    PropData * data = &pi->next->data;
    PROP_ItrAdvance(pi);
    return data;
}

STATIC Bool PROP_ItrHasNext(Iterator * itr)
{
    PropIterator * pi = CAST(itr,PropIterator,itr);
    return BoolValue(pi->next != NULL);
}

STATIC Bool PROP_ItrRemove(Iterator * itr)
{
    PropIterator * pi = CAST(itr,PropIterator,itr);
    PROP_RemoveEntry(pi->entry); 
    pi->entry = NULL;
    return True;
}

STATIC void PROP_ItrFree(Iterator * itr)
{
    PropIterator * pi = CAST(itr,PropIterator,itr);
    MEM_Free(pi);
}
