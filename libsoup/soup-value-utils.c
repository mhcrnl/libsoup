/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-type-utils.c: GValue and GType-related utilities
 *
 * Copyright (C) 2007 Red Hat, Inc.
 */

#include "soup-value-utils.h"

#include <string.h>

static void
soup_value_hash_value_free (gpointer val)
{
	g_value_unset (val);
	g_free (val);
}

/**
 * soup_value_hash_new:
 *
 * Creates a #GHashTable whose keys are strings and whose values
 * are #GValue.
 **/
GHashTable *
soup_value_hash_new (void)
{
	return g_hash_table_new_full (g_str_hash, g_str_equal,
				      g_free, soup_value_hash_value_free);
}

/**
 * soup_value_hash_insert_value:
 * @hash: a value hash
 * @key: the key
 * @value: a value
 *
 * Inserts @value into @hash. (Unlike with g_hash_table_insert(), both
 * the key and the value are copied).
 **/
void
soup_value_hash_insert_value (GHashTable *hash, const char *key, GValue *value)
{
	GValue *copy = g_new0 (GValue, 1);

	g_value_init (copy, G_VALUE_TYPE (value));
	g_value_copy (value, copy);
	g_hash_table_insert (hash, g_strdup (key), copy);
}

/**
 * soup_value_hash_insert:
 * @hash: a value hash
 * @key: the key
 * @type: a #GType
 * @...: a value of type @type
 *
 * Inserts the provided value of type @type into @hash. (Unlike with
 * g_hash_table_insert(), both the key and the value are copied).
 **/
void
soup_value_hash_insert (GHashTable *hash, const char *key, GType type, ...)
{
	va_list args;
	GValue val;

	va_start (args, type);
	SOUP_VALUE_SETV (&val, type, args);
	va_end (args);
	soup_value_hash_insert_value (hash, key, &val);
}

/**
 * soup_value_hash_lookup:
 * @hash: a value hash
 * @key: the key to look up
 * @type: a #GType
 * @...: a value of type pointer-to-@type
 *
 * Looks up @key in @hash and stores its value into the provided
 * location.
 *
 * Return value: %TRUE if @hash contained a value with key @key and
 * type @type, %FALSE if not.
 **/
gboolean
soup_value_hash_lookup (GHashTable *hash, const char *key, GType type, ...)
{
	va_list args;
	GValue *value;

	value = g_hash_table_lookup (hash, key);
	if (!value || !G_VALUE_HOLDS (value, type))
		return FALSE;

	va_start (args, type);
	SOUP_VALUE_GETV (value, type, args);
	va_end (args);

	return TRUE;
}


/**
 * soup_value_array_from_args:
 * @args: arguments to create a #GValueArray from
 *
 * Creates a #GValueArray from the provided arguments, which must
 * consist of pairs of a #GType and a value of that type, terminated
 * by %G_TYPE_INVALID. (The array will contain copies of the provided
 * data rather than pointing to the passed-in data directly.)
 *
 * Return value: a new #GValueArray, or %NULL if an error occurred.
 **/
GValueArray *
soup_value_array_from_args (va_list args)
{
	GValueArray *array;
	GType type;
	GValue val;

	array = g_value_array_new (1);
	while ((type = va_arg (args, GType)) != G_TYPE_INVALID) {
		SOUP_VALUE_SETV (&val, type, args);
		g_value_array_append (array, &val);
	}
	return array;
}

/**
 * soup_value_array_to_args:
 * @array: a #GValueArray
 * @args: arguments to extract @array into
 *
 * Extracts a #GValueArray into the provided arguments, which must
 * consist of pairs of a #GType and a value of pointer-to-that-type,
 * terminated by %G_TYPE_INVALID. The returned values will point to the
 * same memory as the values in the array.
 *
 * Return value: success or failure
 **/
gboolean
soup_value_array_to_args (GValueArray *array, va_list args)
{
	GType type;
	GValue *value;
	int i;

	for (i = 0; i < array->n_values; i++) {
		type = va_arg (args, GType);
		if (type == G_TYPE_INVALID)
			return FALSE;
		value = g_value_array_get_nth (array, i);
		if (!G_VALUE_HOLDS (value, type))
			return FALSE;
		SOUP_VALUE_GETV (value, type, args);
	}
	return TRUE;
}

/**
 * soup_value_array_insert:
 * @array: a #GValueArray
 * @index_: the index to insert at
 * @type: a #GType
 * @...: a value of type @type
 *
 * Inserts the provided value of type @type into @array as with
 * g_value_array_insert(). (The provided data is copied rather than
 * being inserted directly.)
 **/
void
soup_value_array_insert (GValueArray *array, guint index_, GType type, ...)
{
	va_list args;
	GValue val;

	va_start (args, type);
	SOUP_VALUE_SETV (&val, type, args);
	va_end (args);
	g_value_array_insert (array, index_, &val);
}

/**
 * soup_value_array_append:
 * @array: a #GValueArray
 * @type: a #GType
 * @...: a value of type @type
 *
 * Appends the provided value of type @type to @array as with
 * g_value_array_append(). (The provided data is copied rather than
 * being inserted directly.)
 **/
void
soup_value_array_append (GValueArray *array, GType type, ...)
{
	va_list args;
	GValue val;

	va_start (args, type);
	SOUP_VALUE_SETV (&val, type, args);
	va_end (args);
	g_value_array_append (array, &val);
}

/**
 * soup_value_array_get_nth:
 * @array: a #GValueArray
 * @index_: the index to look up
 * @type: a #GType
 * @...: a value of type pointer-to-@type
 *
 * Gets the @index_ element of @array and stores its value into the
 * provided location.
 *
 * Return value: %TRUE if @array contained a value with index @index_
 * and type @type, %FALSE if not.
 **/
gboolean
soup_value_array_get_nth (GValueArray *array, guint index_, GType type, ...)
{
	GValue *value;
	va_list args;

	value = g_value_array_get_nth (array, index_);
	if (!value || !G_VALUE_HOLDS (value, type))
		return FALSE;

	va_start (args, type);
	SOUP_VALUE_GETV (value, type, args);
	va_end (args);
	return TRUE;
}


static GByteArray *
soup_byte_array_copy (GByteArray *ba)
{
	GByteArray *copy;

	copy = g_byte_array_sized_new (ba->len);
	g_byte_array_append (copy, ba->data, ba->len);
	return copy;
}

static void
soup_byte_array_free (GByteArray *ba)
{
	g_byte_array_free (ba, TRUE);
}

GType
soup_byte_array_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static (
			g_intern_static_string ("GByteArray"),
			(GBoxedCopyFunc)soup_byte_array_copy,
			(GBoxedFreeFunc)soup_byte_array_free);
	}
	return type;
}