/*
 * File functions
 *
 * Copyright (c) 2010, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined( _LIBWTCDB_INTERNAL_FILE_H )
#define _LIBWTCDB_INTERNAL_FILE_H

#include <common.h>
#include <types.h>

#include <liberror.h>

#include "libwtcdb_array_type.h"
#include "libwtcdb_extern.h"
#include "libwtcdb_io_handle.h"
#include "libwtcdb_libbfio.h"
#include "libwtcdb_types.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libwtcdb_internal_file libwtcdb_internal_file_t;

struct libwtcdb_internal_file
{
	/* The items array
	 */
	libwtcdb_array_t *items;

	/* The IO handle
	 */
	libwtcdb_io_handle_t *io_handle;

	/* The file IO handle
	 */
	libbfio_handle_t *file_io_handle;

	/* Value to indicate if the file IO handle was created inside the library
	 */
	uint8_t file_io_handle_created_in_library;
};

LIBWTCDB_EXTERN int libwtcdb_file_initialize(
                     libwtcdb_file_t **file,
                     liberror_error_t **error );

LIBWTCDB_EXTERN int libwtcdb_file_free(
                     libwtcdb_file_t **file,
                     liberror_error_t **error );

LIBWTCDB_EXTERN int libwtcdb_file_signal_abort(
                     libwtcdb_file_t *file,
                     liberror_error_t **error );

LIBWTCDB_EXTERN int libwtcdb_file_open(
                     libwtcdb_file_t *file,
                     const char *filename,
                     int access_flags,
                     liberror_error_t **error );

#if defined( HAVE_WIDE_CHARACTER_TYPE )
LIBWTCDB_EXTERN int libwtcdb_file_open_wide(
                     libwtcdb_file_t *file,
                     const wchar_t *filename,
                     int access_flags,
                     liberror_error_t **error );
#endif

LIBWTCDB_EXTERN int libwtcdb_file_open_file_io_handle(
                     libwtcdb_file_t *file,
                     libbfio_handle_t *file_io_handle,
                     int access_flags,
                     liberror_error_t **error );

LIBWTCDB_EXTERN int libwtcdb_file_close(
                     libwtcdb_file_t *file,
                     liberror_error_t **error );

int libwtcdb_file_open_read(
     libwtcdb_internal_file_t *internal_file,
     liberror_error_t **error );

LIBWTCDB_EXTERN int libwtcdb_file_get_number_of_items(
                     libwtcdb_file_t *file,
                     int *number_of_items,
                     liberror_error_t **error );

LIBWTCDB_EXTERN int libwtcdb_file_get_item(
                     libwtcdb_file_t *file,
                     int item_index,
                     libwtcdb_item_t **item,
                     liberror_error_t **error );

#if defined( __cplusplus )
}
#endif

#endif
