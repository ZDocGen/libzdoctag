// zdoctag.h - Defines a struct that represents the zdoc tag structure.
// <tagname (string | zdoctag)*>
//
// [[[WHEN INCLUDING THIS FILE, MAKE SURE TO HAVE *ONE* #include PRECEDED BY
//  * THE FOLLOWING:
//  *     #define ZDOCTAG_IMPL
//  * This way the actual code can be included into your final binary once.
//  *     #define ZDOCTAG_IMPL
//  *     #include "zdoctag.h"
//  * Enjoy!
// ]]]
#ifndef ZDOCTAG_H
	#define ZDOCTAG_H
	#ifdef __cplusplus
extern "C" {
	#endif

	#include <stdbool.h>
	#include <stddef.h>
	#include <stdio.h>
	#include <stdlib.h>


enum ZDocTagDataType {
	ZDOC_TAG,
	ZDOC_STRING,
};

typedef struct ZDocTag ZDocTag;

struct ZDocTagChild {
	enum ZDocTagDataType unionTag;
	union {
		ZDocTag const *childTag;
		char const *childStr;
	} data;
};

struct ZDocTag {
	char const *tagName;
	size_t capacity;
	size_t numChildren;
	struct ZDocTagChild *children;
};


ZDocTag *zdoctag_new(char const *const tagName, size_t initialCapacity);
ZDocTag *zdoctag_null();

int zdoctag_add_child_string(ZDocTag *const self, char const *const toAdd);
int zdoctag_add_child_zdoctag(ZDocTag *const self, ZDocTag const *toAdd);

bool zdoctag_is_null(ZDocTag const *const self);

bool zdoctag_print(ZDocTag const *const self, size_t const print_indent);
void zdoctag_clean(ZDocTag *self);


	#ifdef ZDOCTAG_IMPL
ZDocTag *zdoctag_new(char const *const tagName, size_t initialCapacity)
{
	ZDocTag *zdt = malloc(sizeof(ZDocTag));
	zdt->tagName = tagName;
	zdt->capacity = initialCapacity;
	zdt->numChildren = 0;
	if (initialCapacity == 0) {
		zdt->children = NULL;
	} else {
		zdt->children = malloc(sizeof(ZDocTag) * initialCapacity);
	}

	return zdt;
}

ZDocTag *zdoctag_null()
{
	return zdoctag_new(NULL, 0);
}

int zdoctag_add_child_string(ZDocTag *const self, char const *const toAdd)
{
	if (++self->numChildren > self->capacity) {
		self->capacity *= 2;

		void *newPtr = realloc(self->children,
		                       sizeof(struct ZDocTagChild)
				       * self->capacity);
		if (newPtr == NULL) {
			perror("Failed to realloc zdoctag child list");
			return 1;
		} else {
			self->children = newPtr;
		}
	}

	self->children[self->numChildren - 1] = (struct ZDocTagChild) {
		.unionTag = ZDOC_STRING,
		.data.childStr = toAdd
	};
	return 0;
}

int zdoctag_add_child_zdoctag(ZDocTag *const self, ZDocTag const *toAdd)
{
	if (++self->numChildren > self->capacity) {
		self->capacity *= 2;

		void *newPtr = realloc(self->children,
		                       sizeof(struct ZDocTagChild)
				       * self->capacity);
		if (newPtr == NULL) {
			perror("Failed to realloc zdoctag child list");
			return 1;
		} else {
			self->children = newPtr;
		}
	}

	self->children[self->numChildren - 1] = (struct ZDocTagChild) {
		.unionTag = ZDOC_TAG,
		.data.childTag = toAdd
	};
	return 0;
}

bool zdoctag_is_null(ZDocTag const *const self)
{
	return (self->tagName == NULL);
}

bool zdoctag_print(ZDocTag const *const self, size_t const print_indent)
{
	if (!zdoctag_is_null(self)) {
		if (print_indent > 0)
			printf("\n");
		for (size_t i = 0; i < print_indent; i++)
			printf("\t");
		printf("<%s", self->tagName);
		if (self->children == NULL) {
			for (size_t i = 0; i < print_indent; i++)
				printf("\t");
			printf(">");
			return false;
		}

		bool needIndentClosingSym = false;
		for (size_t i = 0; i < self->numChildren; i++) {
			struct ZDocTagChild child = self->children[i];
			switch (child.unionTag) {
			case ZDOC_STRING:
				printf(" \"%s\"", child.data.childStr);
				continue;
			case ZDOC_TAG:
				needIndentClosingSym |=
					zdoctag_print(child.data.childTag,
						      print_indent + 1);
				continue;
			}
		}
		if (needIndentClosingSym) {
			for (size_t i = 0; i < print_indent; i++) {
				printf("\t");
			}
			printf(">\n");
			return true;
		} else {
			printf(">\n");
			return true;
		}
	}
	return false;
}

void zdoctag_clean(ZDocTag *self)
{
	if (self->children == NULL)
		goto skipChildren;

	for (size_t i = 0; i < self->numChildren; i++) {
		struct ZDocTagChild child = self->children[i];
		switch (child.unionTag) {
		case ZDOC_STRING:
			free((void *) child.data.childStr);
			break;
		case ZDOC_TAG:
			zdoctag_clean((ZDocTag *) child.data.childTag);
			break;
		}
	}

	free(self->children);

skipChildren:
	if (self->tagName == NULL)
		goto skipTagName;
	free((void *) self->tagName);

skipTagName:
	free(self);
}

		#undef ZDOCTAG_IMPL
	#endif
	#ifdef __cplusplus
} /* extern "C" */
	#endif
#endif
