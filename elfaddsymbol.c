#include <err.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <string.h>

void add_data_to_section( void * new_data, size_t new_size, Elf_Data * dat )
{
	int len = dat->d_size + new_size;
	dat->d_buf = realloc( dat->d_buf, len );
	memcpy( dat->d_buf + dat->d_size, new_data, new_size );
	dat->d_size = len;
	elf_flagdata( dat, ELF_C_SET, ELF_F_DIRTY );
}

int main( int argc, char** argv )
{
	if( argc < 3 )
	{
		fprintf( stderr, "Usage: %s <library> <new_symbol>\n", argv[0] );
		exit(1);
	}
	Elf * e;
	int fd = open (argv[1], O_RDWR );
	elf_version( EV_CURRENT );
	e = elf_begin( fd, ELF_C_RDWR, NULL);

	char * strtab = NULL;
	const char * new_sym = argv[2];
	int new_sym_strtab = 0;

	Elf_Scn * sec = NULL;
	while( sec = elf_nextscn( e, sec ) )
	{
		Elf32_Shdr * shdr =  elf32_getshdr( sec );
		if( !shdr )
		{
			printf("ERR: %s\n", elf_errmsg(-1));
			continue;
		}
		if (shdr->sh_type == SHT_STRTAB )
		{
			Elf_Data * dat = NULL;
			while( dat = elf_getdata( sec, dat ) )
			{
				strtab = dat->d_buf;
				new_sym_strtab = dat->d_size; //last byte will be new string
				/* Include NULL term in string copy */
				add_data_to_section( (void*)new_sym, strlen( new_sym ) + 1, dat );
				break;
			}
			break;
		}
	}
	sec = NULL;
	while( sec = elf_nextscn( e, sec ) )
	{
		Elf32_Shdr * shdr =  elf32_getshdr( sec );
		if( !shdr )
		{
			printf("ERR: %s\n", elf_errmsg(-1));
			continue;
		}
		if( shdr->sh_type == SHT_DYNSYM )
		{
			Elf_Data * dat = NULL;
			while( dat = elf_getdata( sec, dat ) )
			{
				/*
				Elf32_Dyn * dyn = (Elf32_Dyn*)dat->d_buf;
				int i;
				for( i = 0; i < dat->d_size/sizeof(dyn[0]); i++ )
				{
					printf( "%d : 0x%x\n", dyn[i].d_tag, dyn[i].d_un.d_val);
				}
				*/
				Elf32_Sym sym = {0};
//sym.st_name, sym.st_value, sym.st_size, sym.st_info, sym.st_other, sym.st_shndx;
				sym.st_name = new_sym_strtab;
				sym.st_info = 16;
				add_data_to_section( (void*)&sym, sizeof(sym), dat );

				Elf32_Sym * syms = (Elf32_Sym*)dat->d_buf;
				int i;
				for( i = 0; i < dat->d_size/sizeof(syms[0]); i++ )
				{
					Elf32_Sym sym = syms[i];
					printf( "%d : 0x%x 0x%x 0x%x 0x%x 0x%X\n", 
					sym.st_name, sym.st_value, sym.st_size, sym.st_info, sym.st_other, sym.st_shndx );
				}

			}
		}
	}
	elf_update( e, ELF_C_WRITE );
	elf_end( e );
	close ( fd );

	return 1;
}
