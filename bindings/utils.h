#ifndef AS_BINDING_UTILS_H
#define AS_BINDING_UTILS_H
#include <core/object.h>
#include <core/os/memory.h>
#include <angelscript.h>

namespace asb {

static void * as_memalloc(size_t size) {
	return memalloc(size);
}

static void as_memfree(void* ptr) {
	memrealloc(ptr, 0);
}

/**
 * Default constructor for value types
 */
template<class T>
void value_constructor(T *p_this) {
	memnew_placement(p_this, T);
}

template<class T, class P>
void value_constructor(P p1, T *p_this) {
	memnew_placement(p_this, T(p1));
}

template<class T, class P1, class P2>
void value_constructor(P1 p1, P2 p2, T *p_this) {
	memnew_placement(p_this, T(p1, p2));
}

template<class T, class P1, class P2, class P3>
void value_constructor(P1 p1, P2 p2, P3 p3, T *p_this) {
	memnew_placement(p_this, T(p1, p2, p3));
}

template<class T, class P1, class P2, class P3, class P4>
void value_constructor(P1 p1, P2 p2, P3 p3, P4 p4, T *p_this) {
	memnew_placement(p_this, T(p1, p2, p3, p4));
}

template<class T, class P1, class P2, class P3, class P4, class P5>
void value_constructor(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, T *p_this) {
	memnew_placement(p_this, T(p1, p2, p3, p4, p5));
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6>
void value_constructor(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, T *p_this) {
	memnew_placement(p_this, T(p1, p2, p3, p4, p5, p6));
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
void value_constructor(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, T *p_this) {
	memnew_placement(p_this, T(p1, p2, p3, p4, p5, p6, p7));
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8>
void value_constructor(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, T *p_this) {
	memnew_placement(p_this, T(p1, p2, p3, p4, p5, p6, p7, p8));
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9>
void value_constructor(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, T *p_this) {
	memnew_placement(p_this, T(p1, p2, p3, p4, p5, p6, p7, p8, p9));
}

/**
 * Destructor of value types
 */
template<class T>
void value_desctructor(T *p_this) {
	if(p_this) p_this->~T();
}

/**
 * Default copy constructor for value types
 */
template<class T>
void value_copy_constructor(const T &other, T *p_this) {
	memnew_placement(p_this, T(other));
}

/**
 * Copy constructor from another value type
 */
template<class T, class P>
void value_copy_constructor(const P &other, T *p_this) {
	memnew_placement(p_this, T(other));
}

/**
 * Assignment operator for value types
 */
template<class T, class P>
T& value_op_assign(const P &p_value, T &p_dest) {
	p_dest = p_value;
	return p_dest;
}

/**
 * Type convertion for value types that supported by Variant
 */
template<class T, class P>
static P value_convert(T *p_this) {
	return *p_this;
}


/**
 * Convert pointer to value type
 */
template<class T, class P>
static P ptr_value_convert(T *p_this) {
	return p_this;
}

/**
 * Compare function
 */
template<class T, class P>
int value_compare(const T &self, const P &another) {
	int cmp = 1;
	if( self < another )
		cmp = -1;
	else if( self == another )
		cmp = 0;
	return cmp;
}

/**
 * Construct objects
 */
template<class T>
static T* object_factory() {
	return memnew(T);
}

/**
 * free objects
 */
template<class T>
void object_free(T* p_this) {
	if (p_this) memdelete(p_this);
}

/**
 * Assignment operator
 */
template<class T, class P>
T& op_assign(P p_value, T &p_dest) {
	p_dest = p_value;
	return p_dest;
}

/**
 * Type cast for object types
 */
template<class T, class P>
P* object_convert(T* p_this) {
	return Object::cast_to<P>(p_this);
}


#ifdef TOOLS_ENABLED

String get_binding_script_content();

#endif

}
#endif // AS_BINDING_UTILS_H
