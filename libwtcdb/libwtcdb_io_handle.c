/*
 * Input/Output (IO) handle functions
 *
 * Copyright (C) 2010-2016, Joachim Metz <joachim.metz@gmail.com>
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

#include <common.h>
#include <byte_stream.h>
#include <memory.h>
#include <narrow_string.h>
#include <system_string.h>
#include <types.h>
#include <wide_string.h>

#include "libwtcdb_crc.h"
#include "libwtcdb_debug.h"
#include "libwtcdb_definitions.h"
#include "libwtcdb_io_handle.h"
#include "libwtcdb_item_value.h"
#include "libwtcdb_libbfio.h"
#include "libwtcdb_libcdata.h"
#include "libwtcdb_libcerror.h"
#include "libwtcdb_libcnotify.h"
#include "libwtcdb_libfdatetime.h"
#include "libwtcdb_libfvalue.h"
#include "libwtcdb_libuna.h"

#include "wtcdb_cache_entry.h"
#include "wtcdb_file_header.h"
#include "wtcdb_index_entry.h"

const uint8_t *wtcdb_cache_file_signature = (uint8_t *) "CMMM";
const uint8_t *wtcdb_index_file_signature = (uint8_t *) "IMMM";

/* Creates an IO handle
 * Make sure the value io_handle is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libwtcdb_io_handle_initialize(
     libwtcdb_io_handle_t **io_handle,
     libcerror_error_t **error )
{
	static char *function = "libwtcdb_io_handle_initialize";

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( *io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid IO handle value already set.",
		 function );

		return( -1 );
	}
	*io_handle = memory_allocate_structure(
	              libwtcdb_io_handle_t );

	if( *io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create IO handle.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *io_handle,
	     0,
	     sizeof( libwtcdb_io_handle_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear file.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( *io_handle != NULL )
	{
		memory_free(
		 *io_handle );

		*io_handle = NULL;
	}
	return( -1 );
}

/* Frees an IO handle
 * Returns 1 if successful or -1 on error
 */
int libwtcdb_io_handle_free(
     libwtcdb_io_handle_t **io_handle,
     libcerror_error_t **error )
{
	static char *function = "libwtcdb_io_handle_free";

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( *io_handle != NULL )
	{
		memory_free(
		 *io_handle );

		*io_handle = NULL;
	}
	return( 1 );
}

/* Clears the IO handle
 * Returns 1 if successful or -1 on error
 */
int libwtcdb_io_handle_clear(
     libwtcdb_io_handle_t *io_handle,
     libcerror_error_t **error )
{
	static char *function = "libwtcdb_io_handle_clear";

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     io_handle,
	     0,
	     sizeof( libwtcdb_io_handle_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear IO handle.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Reads the file header
 * Returns 1 if successful or -1 on error
 */
int libwtcdb_io_handle_read_file_header(
     libwtcdb_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     uint32_t *first_entry_offset,
     uint32_t *available_cache_entry_offset,
     uint32_t *number_of_entries,
     libcerror_error_t **error )
{
	uint8_t file_header_data[ 24 ];

	static char *function     = "libwtcdb_io_handle_read_file_header";
	ssize_t read_count        = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint8_t *trailing_data    = NULL;
	size_t trailing_data_size = 0;
	uint32_t value_32bit      = 0;
#endif

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( first_entry_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid first entry offset.",
		 function );

		return( -1 );
	}
	if( available_cache_entry_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid available cache entry offset.",
		 function );

		return( -1 );
	}
	if( number_of_entries == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid number of entries.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: reading file header at offset: 0 (0x00000000)\n",
		 function );
	}
#endif
	if( libbfio_handle_seek_offset(
	     file_io_handle,
	     0,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek file header offset: 0.",
		 function );

		return( -1 );
	}
	read_count = libbfio_handle_read_buffer(
	              file_io_handle,
	              file_header_data,
	              24,
	              error );

	if( read_count != (ssize_t) 24 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read file header.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: file header:\n",
		 function );
		libcnotify_print_data(
		 file_header_data,
		 24,
		 0 );
	}
#endif
	if( memory_compare(
	     file_header_data,
	     wtcdb_cache_file_signature,
	     4 ) == 0 )
	{
		io_handle->file_type = LIBWTCDB_FILE_TYPE_CACHE;
	}
	else if( memory_compare(
	          file_header_data,
	          wtcdb_index_file_signature,
	          4 ) == 0 )
	{
		io_handle->file_type = LIBWTCDB_FILE_TYPE_INDEX;
	}
	else
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: invalid file signature.",
		 function );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 &( file_header_data[ 4 ] ),
	 io_handle->version );

	if( io_handle->file_type == LIBWTCDB_FILE_TYPE_CACHE )
	{
		byte_stream_copy_to_uint32_little_endian(
		 ( (wtcdb_cache_file_header_t *) file_header_data )->first_cache_entry_offset,
		 *first_entry_offset );

		byte_stream_copy_to_uint32_little_endian(
		 ( (wtcdb_cache_file_header_t *) file_header_data )->available_cache_entry_offset,
		 *available_cache_entry_offset );

		byte_stream_copy_to_uint32_little_endian(
		 ( (wtcdb_cache_file_header_t *) file_header_data )->number_of_cache_entries,
		 *number_of_entries );
	}
	else if( io_handle->file_type == LIBWTCDB_FILE_TYPE_INDEX )
	{
		*first_entry_offset = sizeof( wtcdb_index_file_header_t );

		byte_stream_copy_to_uint32_little_endian(
		 ( (wtcdb_index_file_header_t *) file_header_data )->number_of_index_entries,
		 *number_of_entries );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: signature\t\t\t\t\t: %c%c%c%c\n",
		 function,
		 file_header_data[ 0 ],
		 file_header_data[ 1 ],
		 file_header_data[ 2 ],
		 file_header_data[ 3 ] );

		libcnotify_printf(
		 "%s: version\t\t\t\t\t: %" PRIu32 "\n",
		 function,
		 io_handle->version );

		if( io_handle->file_type == LIBWTCDB_FILE_TYPE_CACHE )
		{
			byte_stream_copy_to_uint32_little_endian(
			 ( (wtcdb_cache_file_header_t *) file_header_data )->cache_type,
			 value_32bit );
			libcnotify_printf(
			 "%s: cache type\t\t\t\t\t: %" PRIu32 "\n",
			 function,
			 value_32bit );

			libcnotify_printf(
			 "%s: first cache entry offset\t\t\t: 0x%08" PRIx32 "\n",
			 function,
			 *first_entry_offset );

			libcnotify_printf(
			 "%s: available cache entry offset\t\t: 0x%08" PRIx32 "\n",
			 function,
			 *available_cache_entry_offset );

			libcnotify_printf(
			 "%s: number of cache entries\t\t\t: %" PRIu32 "\n",
			 function,
			 *number_of_entries );
		}
		else if( io_handle->file_type == LIBWTCDB_FILE_TYPE_INDEX )
		{
			byte_stream_copy_to_uint32_little_endian(
			 ( (wtcdb_index_file_header_t *) file_header_data )->unknown1,
			 value_32bit );
			libcnotify_printf(
			 "%s: unknown1\t\t\t\t\t: 0x%08" PRIx32 "\n",
			 function,
			 value_32bit );

			byte_stream_copy_to_uint32_little_endian(
			 ( (wtcdb_index_file_header_t *) file_header_data )->number_of_index_entries_used,
			 value_32bit );
			libcnotify_printf(
			 "%s: number of index entries used\t\t: %" PRIu32 "\n",
			 function,
			 value_32bit );

			libcnotify_printf(
			 "%s: number of index entries\t\t\t: %" PRIu32 "\n",
			 function,
			 *number_of_entries );

			byte_stream_copy_to_uint32_little_endian(
			 ( (wtcdb_index_file_header_t *) file_header_data )->unknown2,
			 value_32bit );
			libcnotify_printf(
			 "%s: unknown2\t\t\t\t\t: 0x%08" PRIx32 "\n",
			 function,
			 value_32bit );
		}
		libcnotify_printf(
		 "\n" );
	}
#endif
	if( ( io_handle->version != 20 )
	 && ( io_handle->version != 21 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported version: %" PRIu32 ".",
		 function,
		 io_handle->version );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( *first_entry_offset > 24 )
		{
			trailing_data_size = (size_t) ( *first_entry_offset - 24 );

			trailing_data = (uint8_t *) memory_allocate(
			                             sizeof( uint8_t ) * trailing_data_size );

			if( trailing_data == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_MEMORY,
				 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
				 "%s: unable to create trailing data.",
				 function );

				return( -1 );
			}
			read_count = libbfio_handle_read_buffer(
				      file_io_handle,
				      trailing_data,
				      trailing_data_size,
				      error );

			if( read_count != (ssize_t) trailing_data_size )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read trailing data.",
				 function );

				memory_free(
				 trailing_data );

				return( -1 );
			}
			libcnotify_printf(
			 "%s: trailing data:\n",
			 function );
			libcnotify_print_data(
			 trailing_data,
			 trailing_data_size,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );

			memory_free(
			 trailing_data );
		}
	}
#endif
	return( 1 );
}

/* Reads the items into the items array
 * Returns 1 if successful or -1 on error
 */
int libwtcdb_io_handle_read_items(
     libwtcdb_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     uint32_t first_entry_offset,
     uint32_t available_cache_entry_offset,
     uint32_t number_of_items,
     libcdata_array_t *items_array,
     libcerror_error_t **error )
{
	libwtcdb_item_value_t *item_value           = NULL;
	uint8_t *item_entry_data                    = NULL;
	const char *type_string                     = NULL;
	static char *function                       = "libwtcdb_io_handle_read_items";
	size_t item_entry_data_size                 = 0;
	ssize_t read_count                          = 0;
	uint64_t calculated_crc                     = 0;
	uint64_t stored_data_crc                    = 0;
	uint64_t stored_header_crc                  = 0;
	uint32_t data_size                          = 0;
	uint32_t item_iterator                      = 0;
	uint32_t padding_size                       = 0;
	uint32_t cache_entry_offset                 = 0;
	uint32_t cache_entry_size                   = 0;
	int item_entry_index                        = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	system_character_t filetime_string[ 32 ];

	libfdatetime_filetime_t *filetime           = NULL;
	libcstring_system_character_t *value_string = NULL;
	uint8_t *padding_data                       = NULL;
	size_t value_string_size                    = 0;
	uint64_t value_64bit                        = 0;
	uint32_t value_32bit                        = 0;
	int result                                  = 0;
#endif

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( ( io_handle->file_type != LIBWTCDB_FILE_TYPE_CACHE )
	 && ( io_handle->file_type != LIBWTCDB_FILE_TYPE_INDEX ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported file type.",
		 function );

		return( -1 );
	}
	if( ( io_handle->version != 20 )
	 && ( io_handle->version != 21 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported version: %" PRIu32 ".",
		 function,
		 io_handle->version );

		return( -1 );
	}
	if( items_array == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid items array.",
		 function );

		return( -1 );
	}
	if( io_handle->file_type == LIBWTCDB_FILE_TYPE_CACHE )
	{
		if( io_handle->version == 20 )
		{
			item_entry_data_size = sizeof( wtcdb_cache_entry_vista_t );
		}
		else if( io_handle->version == 21 )
		{
			item_entry_data_size = sizeof( wtcdb_cache_entry_win7_t );
		}
		type_string = "cache";
	}
	else if( io_handle->file_type == LIBWTCDB_FILE_TYPE_INDEX )
	{
		if( io_handle->version == 20 )
		{
			item_entry_data_size = sizeof( wtcdb_index_entry_vista_t );
		}
		else if( io_handle->version == 21 )
		{
			item_entry_data_size = sizeof( wtcdb_index_entry_win7_t );
		}
		type_string = "index";
	}
	item_entry_data = (uint8_t *) memory_allocate(
	                               sizeof( uint8_t ) * item_entry_data_size );

	if( item_entry_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create item entry data.",
		 function );

		goto on_error;
	}
	for( item_iterator = 0;
	     item_iterator < number_of_items;
	     item_iterator++ )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: reading %s entry: %" PRIu32 " at offset: %" PRIu32 " (0x%08" PRIx32 ")\n",
			 function,
			 type_string,
			 item_iterator,
			 first_entry_offset,
			 first_entry_offset );
		}
#endif
		if( io_handle->file_type == LIBWTCDB_FILE_TYPE_CACHE )
		{
			/* The skip cached data
			 */
			if( libbfio_handle_seek_offset(
			     file_io_handle,
			     (off64_t) first_entry_offset,
			     SEEK_SET,
			     error ) == -1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_SEEK_FAILED,
				 "%s: unable to seek cache entry: %" PRIu32 " offset: %" PRIu32 ".",
				 function,
				 item_iterator,
				 first_entry_offset );

				goto on_error;
			}
		}
		read_count = libbfio_handle_read_buffer(
			      file_io_handle,
			      item_entry_data,
			      item_entry_data_size,
			      error );

		if( read_count != (ssize_t) item_entry_data_size )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read %s entry: %" PRIu32 " data.",
			 function,
			 type_string,
			 item_iterator );

			goto on_error;
		}
		cache_entry_offset = item_entry_data_size;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: %s entry: %04" PRIu32 " data:\n",
			 function,
			 type_string,
			 item_iterator );
			libcnotify_print_data(
			 item_entry_data,
			 item_entry_data_size,
			 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
		}
#endif
		if( libwtcdb_item_value_initialize(
		     &item_value,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create item value.",
			 function );

			goto on_error;
		}
		if( io_handle->file_type == LIBWTCDB_FILE_TYPE_CACHE )
		{
			if( memory_compare(
			     ( (wtcdb_cache_entry_vista_t *) item_entry_data )->signature,
			     wtcdb_cache_file_signature,
			     4 ) != 0 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
				 "%s: invalid cache entry: %" PRIu32 " signature.",
				 function,
				 item_iterator );

				goto on_error;
			}
			byte_stream_copy_to_uint32_little_endian(
			 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->size,
			 cache_entry_size );

			if( io_handle->version == 20 )
			{
				byte_stream_copy_to_uint32_little_endian(
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->identifier_string_size,
				 item_value->identifier_size );

				byte_stream_copy_to_uint32_little_endian(
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->padding_size,
				 padding_size );

				byte_stream_copy_to_uint32_little_endian(
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->data_size,
				 data_size );

				byte_stream_copy_to_uint64_little_endian(
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->header_checksum,
				 stored_header_crc );

				byte_stream_copy_to_uint64_little_endian(
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->data_checksum,
				 stored_data_crc );
			}
			else if( io_handle->version == 21 )
			{
				byte_stream_copy_to_uint32_little_endian(
				 ( (wtcdb_cache_entry_win7_t *) item_entry_data )->identifier_string_size,
				 item_value->identifier_size );

				byte_stream_copy_to_uint32_little_endian(
				 ( (wtcdb_cache_entry_win7_t *) item_entry_data )->padding_size,
				 padding_size );

				byte_stream_copy_to_uint32_little_endian(
				 ( (wtcdb_cache_entry_win7_t *) item_entry_data )->data_size,
				 data_size );

				byte_stream_copy_to_uint64_little_endian(
				 ( (wtcdb_cache_entry_win7_t *) item_entry_data )->data_checksum,
				 stored_data_crc );

				byte_stream_copy_to_uint64_little_endian(
				 ( (wtcdb_cache_entry_win7_t *) item_entry_data )->header_checksum,
				 stored_header_crc );
			}
#if defined( HAVE_DEBUG_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " signature\t\t\t: %c%c%c%c\n",
				 function,
				 item_iterator,
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->signature[ 0 ],
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->signature[ 1 ],
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->signature[ 2 ],
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->signature[ 3 ] );

				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " size\t\t\t\t: %" PRIu32 "\n",
				 function,
				 item_iterator,
				 cache_entry_size );

				byte_stream_copy_to_uint64_little_endian(
				 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->entry_hash,
				 value_64bit );
				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " entry hash\t\t\t: 0x%08" PRIx64 "\n",
				 function,
				 item_iterator,
				 value_64bit );

				if( io_handle->version == 20 )
				{
/* TODO make a string of this ?*/
					libcnotify_printf(
					 "%s: cache entry: %04" PRIu32 " file extension\t\t\t: ",
					 function,
					 item_iterator );

					if( ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 0 ] == 0 )
					{
						libcnotify_printf(
						 "(none)" );
					}
					else
					{
						libcnotify_printf(
						 "%c",
						 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 0 ] );
					}
					if( ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 2 ] != 0 )
					{
						libcnotify_printf(
						 "%c",
						 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 2 ] );
					}
					if( ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 4 ] != 0 )
					{
						libcnotify_printf(
						 "%c",
						 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 4 ] );
					}
					if( ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 6 ] != 0 )
					{
						libcnotify_printf(
						 "%c",
						 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->file_extension[ 6 ] );
					}
					libcnotify_printf(
					 "\n" );
				}
				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " identifier string size\t\t: %" PRIu32 "\n",
				 function,
				 item_iterator,
				 item_value->identifier_size );

				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " padding size\t\t\t: %" PRIu32 "\n",
				 function,
				 item_iterator,
				 padding_size );

				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " data size\t\t\t: %" PRIu32 "\n",
				 function,
				 item_iterator,
				 data_size );

				if( io_handle->version == 20 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_cache_entry_vista_t *) item_entry_data )->unknown1,
					 value_32bit );
				}
				else if( io_handle->version == 21 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_cache_entry_win7_t *) item_entry_data )->unknown1,
					 value_32bit );
				}
				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " unknown1\t\t\t: 0x%08" PRIx32 "\n",
				 function,
				 item_iterator,
				 value_32bit );

				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " data checksum\t\t\t: 0x%08" PRIx64 "\n",
				 function,
				 item_iterator,
				 stored_data_crc );

				libcnotify_printf(
				 "%s: cache entry: %04" PRIu32 " header checksum\t\t: 0x%08" PRIx64 "\n",
				 function,
				 item_iterator,
				 stored_header_crc );

				libcnotify_printf(
				 "\n" );
			}
#endif
			if( libwtcdb_crc64_weak_calculate(
			     &calculated_crc,
			     item_entry_data,
			     item_entry_data_size - 8,
			     (uint64_t) -1,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
				 "%s: unable to calculate CRC-64.",
				 function );

				goto on_error;
			}
			if( stored_header_crc != calculated_crc )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: mismatch in header CRC-64 ( 0x%08" PRIx64 " != 0x%08" PRIx64 " ).\n",
					 function,
					 stored_header_crc,
					 calculated_crc );
				}
#endif
			}
			if( item_value->identifier_size > 0 )
			{
				if( ( cache_entry_offset + item_value->identifier_size ) > cache_entry_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
					 "%s: invalid identifier string size value exceeds cache entry size.",
					 function );

					goto on_error;
				}
				item_value->identifier = (uint8_t *) memory_allocate(
								      sizeof( uint8_t ) * item_value->identifier_size );

				if( item_value->identifier == NULL )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_MEMORY,
					 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
					 "%s: unable to create identifier.",
					 function );

					goto on_error;
				}
				read_count = libbfio_handle_read_buffer(
					      file_io_handle,
					      item_value->identifier,
					      item_value->identifier_size,
					      error );

				if( read_count != (ssize_t) item_value->identifier_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read cache entry: %" PRIu32 " identifier string data.",
					 function,
					 item_iterator );

					goto on_error;
				}
				cache_entry_offset += item_value->identifier_size;

#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: cache entry: %04" PRIu32 " identifier string data:\n",
					 function,
					 item_iterator );
					libcnotify_print_data(
					 item_value->identifier,
					 item_value->identifier_size,
					 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
				}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
					result = libuna_utf16_string_size_from_utf16_stream(
						  item_value->identifier,
						  item_value->identifier_size,
						  LIBUNA_ENDIAN_LITTLE,
						  &value_string_size,
						  error );
#else
					result = libuna_utf8_string_size_from_utf16_stream(
						  item_value->identifier,
						  item_value->identifier_size,
						  LIBUNA_ENDIAN_LITTLE,
						  &value_string_size,
						  error );
#endif
					if( result != 1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						 "%s: unable to determine size of volume label string.",
						 function );

						goto on_error;
					}
					value_string = system_string_allocate(
					                value_string_size );

					if( value_string == NULL )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_MEMORY,
						 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
						 "%s: unable to create volume label string.",
						 function );

						goto on_error;
					}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
					result = libuna_utf16_string_copy_from_utf16_stream(
						  (libuna_utf16_character_t *) value_string,
						  value_string_size,
						  item_value->identifier,
						  item_value->identifier_size,
						  LIBUNA_ENDIAN_LITTLE,
						  error );
#else
					result = libuna_utf8_string_copy_from_utf16_stream(
						  (libuna_utf8_character_t *) value_string,
						  value_string_size,
						  item_value->identifier,
						  item_value->identifier_size,
						  LIBUNA_ENDIAN_LITTLE,
						  error );
#endif
					if( result != 1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
						 "%s: unable to set volume label string.",
						 function );

						memory_free(
						 value_string );

						goto on_error;
					}
					libcnotify_printf(
					 "%s: cache entry: %04" PRIu32 " identifier string\t\t: %" PRIs_SYSTEM "\n",
					 function,
					 item_iterator,
					 value_string );

					memory_free(
					 value_string );
				}
#endif
				/* TODO remove when value is appended */
				libwtcdb_item_value_free(
				 &item_value,
				 NULL );

				item_value = NULL;
			}
			if( padding_size > 0 )
			{
				if( ( cache_entry_offset + padding_size ) > cache_entry_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
					 "%s: invalid padding size value exceeds cache entry size.",
					 function );

					goto on_error;
				}
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					padding_data = (uint8_t *) memory_allocate(
					                            sizeof( uint8_t ) * padding_size );

					if( padding_data == NULL )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_MEMORY,
						 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
						 "%s: unable to create padding data.",
						 function );

						goto on_error;
					}
					read_count = libbfio_handle_read_buffer(
						      file_io_handle,
						      padding_data,
						      padding_size,
						      error );

					if( read_count != (ssize_t) padding_size )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_IO,
						 LIBCERROR_IO_ERROR_READ_FAILED,
						 "%s: unable to read padding data.",
						 function );

						memory_free(
						 padding_data );

						goto on_error;
					}
					libcnotify_printf(
					 "%s: %s entry: %04" PRIu32 " padding data:\n",
					 function,
					 type_string,
					 item_iterator );
					libcnotify_print_data(
					 padding_data,
					 padding_size,
					 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );

					memory_free(
					 padding_data );
				}
#endif
				cache_entry_offset += padding_size;
			}
			if( data_size > 0 )
			{
				if( ( cache_entry_offset + data_size ) > cache_entry_size )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
					 "%s: invalid data size value exceeds cache entry size.",
					 function );

					goto on_error;
				}
				cache_entry_offset += data_size;

			}
			/* TODO put data offset in runtime cache entry */

			if( first_entry_offset == available_cache_entry_offset )
			{
				break;
			}
			if( cache_entry_offset != cache_entry_size )
			{
fprintf( stderr, "SIZE MISMATCH: %" PRIu32 " %" PRIu32 "n",
 cache_entry_offset,
 cache_entry_size );
			}
			first_entry_offset += cache_entry_size;
		}
		else if( io_handle->file_type == LIBWTCDB_FILE_TYPE_INDEX )
		{
#if defined( HAVE_DEBUG_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				byte_stream_copy_to_uint64_little_endian(
				 ( (wtcdb_index_entry_vista_t *) item_entry_data )->entry_hash,
				 value_64bit );
				libcnotify_printf(
				 "%s: index entry: %04" PRIu32 " entry hash\t\t\t: 0x%08" PRIx64 "\n",
				 function,
				 item_iterator,
				 value_64bit );

				if( libfdatetime_filetime_initialize(
				     &filetime,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to create filetime.",
					 function );

					return( -1 );
				}
				if( libfdatetime_filetime_copy_from_byte_stream(
				     filetime,
				     ( (wtcdb_index_entry_vista_t *) item_entry_data )->modification_time,
				     8,
				     LIBFDATETIME_ENDIAN_LITTLE,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
					 "%s: unable to copy byte stream to filetime.",
					 function );

					libfdatetime_filetime_free(
					 &filetime,
					 NULL );

					return( -1 );
				}
#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
				result = libfdatetime_filetime_copy_to_utf16_string(
					  filetime,
					  (uint16_t *) filetime_string,
					  32,
					  LIBFDATETIME_STRING_FORMAT_TYPE_CTIME | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
					  error );
#else
				result = libfdatetime_filetime_copy_to_utf8_string(
					  filetime,
					  (uint8_t *) filetime_string,
					  32,
					  LIBFDATETIME_STRING_FORMAT_TYPE_CTIME | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
					  error );
#endif
				if( result != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_COPY_FAILED,
					 "%s: unable to copy filetime to string.",
					 function );

					libfdatetime_filetime_free(
					 &filetime,
					 NULL );

					return( -1 );
				}
				if( libfdatetime_filetime_free(
				     &filetime,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
					 "%s: unable to free filetime.",
					 function );

					return( -1 );
				}
				libcnotify_printf(
				 "%s: modification time\t: %" PRIs_SYSTEM " UTC\n\n",
				 filetime_string );

				if( io_handle->version == 20 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_vista_t *) item_entry_data )->flags,
					 value_32bit );
				}
				else if( io_handle->version == 21 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_win7_t *) item_entry_data )->flags,
					 value_32bit );
				}
				libcnotify_printf(
				 "%s: index entry: %04" PRIu32 " flags\t\t\t\t: 0x%08" PRIx32 "\n",
				 function,
				 item_iterator,
				 value_32bit );

				if( io_handle->version == 20 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_vista_t *) item_entry_data )->cache_entry_offset_32,
					 value_32bit );
				}
				else if( io_handle->version == 21 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_win7_t *) item_entry_data )->cache_entry_offset_32,
					 value_32bit );
				}
				libcnotify_printf(
				 "%s: index entry: %04" PRIu32 " cache entry offset 32\t\t: 0x%08" PRIx32 "\n",
				 function,
				 item_iterator,
				 value_32bit );

				if( io_handle->version == 20 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_vista_t *) item_entry_data )->cache_entry_offset_96,
					 value_32bit );
				}
				else if( io_handle->version == 21 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_win7_t *) item_entry_data )->cache_entry_offset_96,
					 value_32bit );
				}
				libcnotify_printf(
				 "%s: index entry: %04" PRIu32 " cache entry offset 96\t\t: 0x%08" PRIx32 "\n",
				 function,
				 item_iterator,
				 value_32bit );

				if( io_handle->version == 20 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_vista_t *) item_entry_data )->cache_entry_offset_256,
					 value_32bit );
				}
				else if( io_handle->version == 21 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_win7_t *) item_entry_data )->cache_entry_offset_256,
					 value_32bit );
				}
				libcnotify_printf(
				 "%s: index entry: %04" PRIu32 " cache entry offset 256\t\t: 0x%08" PRIx32 "\n",
				 function,
				 item_iterator,
				 value_32bit );

				if( io_handle->version == 20 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_vista_t *) item_entry_data )->cache_entry_offset_1024,
					 value_32bit );
				}
				else if( io_handle->version == 21 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_win7_t *) item_entry_data )->cache_entry_offset_1024,
					 value_32bit );
				}
				libcnotify_printf(
				 "%s: index entry: %04" PRIu32 " cache entry offset 1024\t: 0x%08" PRIx32 "\n",
				 function,
				 item_iterator,
				 value_32bit );

				if( io_handle->version == 20 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_vista_t *) item_entry_data )->cache_entry_offset_sr,
					 value_32bit );
				}
				else if( io_handle->version == 21 )
				{
					byte_stream_copy_to_uint32_little_endian(
					 ( (wtcdb_index_entry_win7_t *) item_entry_data )->cache_entry_offset_sr,
					 value_32bit );
				}
				libcnotify_printf(
				 "%s: index entry: %04" PRIu32 " cache entry offset sr\t\t: 0x%08" PRIx32 "\n",
				 function,
				 item_iterator,
				 value_32bit );
			}
#endif
			first_entry_offset += item_entry_data_size;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "\n" );
		}
#endif
/* TODO
		if( libcdata_array_append_entry(
		     items_array,
		     &item_entry_index,
		     (intptr_t *) values_table,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
			 "%s: unable to append item values to items array.",
			 function );

			goto on_error;
		}
		values_table = NULL;
*/
	}
	memory_free(
	 item_entry_data );

	return( 1 );

on_error:
	if( item_value != NULL )
	{
		libwtcdb_item_value_free(
		 &item_value,
		 NULL );
	}
	if( item_entry_data != NULL )
	{
		memory_free(
		 item_entry_data );
	}
	return( -1 );
}

