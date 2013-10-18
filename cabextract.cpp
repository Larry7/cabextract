// hotSync.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//static struct hot_arch_s 
//{
//	int   code;
//	char* name;
//}
//my_arch[] = 
//{
//	0,     "none",
//	103,   "SHx SH3",
//	104,   "SHx SH4",
//	386,   "Intel 386",
//	486,   "Intel 486",
//	586,   "Intel Pentium",
//	601,   "PowerPC 601",
//	603,   "PowerPC 603",
//	604,   "PowerPC 604",
//	620,   "PowerPC 620",
//	821,   "Motorola 821",
//	1824,  "ARM 720",
//	2080,  "ARM 820",
//	2336,  "ARM 920",
//	2577,  "StrongARM",
//	4000,  "MIPS R4000",
//	10003, "Hitachi SH3",
//	10004, "Hitachi SH3E",
//	10005, "Hitachi SH4",
//	21064, "Alpha 21064",
//	70001, "ARM 7TDMI"
//};

char* my_ce[] = 
{
  NULL,
  "\\Program Files",
  "\\Windows",
  "\\Windows\\Desktop",
  "\\Windows\\StartUp",
  "\\My Documents",
  "\\Program Files\\Accessories",
  "\\Program Files\\Communications",
  "\\Program Files\\Games",
  "\\Program Files\\Pocket Outlook",
  "\\Program Files\\Office",
  "\\Windows\\Programs",
  "\\Windows\\Programs\\Accessories",
  "\\Windows\\Programs\\Communications",
  "\\Windows\\Programs\\Games",
  "\\Windows\\Fonts",
  "\\Windows\\Recent",
  "\\Windows\\Favorites"
};

//char* my_hkeys[] = 
//{
//  NULL,
//  "HKEY_CLASSES_ROOT",
//  "HKEY_CURRENT_USER",
//  "HKEY_LOCAL_MACHINE",
//  "HKEY_USERS"
//};

bool get_fname(const std::string& target_path, const std::string& id, std::string& name)
{
	std::vector< std::string > all_matching_files;

	if( boost::filesystem::is_directory( target_path ) && boost::filesystem::exists( target_path ) )
	{
		boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
		for( boost::filesystem::directory_iterator i( target_path ); i != end_itr; ++i )
		{
			// Skip if not a file
			if( !boost::filesystem::is_regular_file( i->status() ) ) 
				continue;

			// Skip if no match
			if( i->path().extension().string() != id ) 
				continue;

			// File matches, store it
			all_matching_files.push_back( i->path().string() );
		}
	}

	size_t len = all_matching_files.size();
	if (len > 0) 
	{
		//use the first one anyway
		name = all_matching_files[0];
	}
	return (len == 1);
}

int main(int argc, char* argv[])
{
	std::string target, output;
	if (argc < 3) 
	{
		target = ".";
		output = "root";
	} 
	else 
	{
		target = std::string(argv[1]);
		output = std::string(argv[2]);
	}

	std::string fname;
	if (!get_fname (target, ".000", fname))
	{
		std::cout << "no header (*.000) file found" << std::endl;
		exit(0);
	}

	//read the header file
	FILE* file_handle = fopen (fname.c_str(), "rb");
	if (NULL != file_handle)
	{
		size_t len = 0x64;
		char buf[256];
		if (len != fread (buf, 1, len, file_handle))
		{
			fclose (file_handle);
			exit (-1);
		}

		//does this the beginning with "MSCE"
		if (buf[0] == 'M' &&
			buf[1] == 'S' &&
			buf[2] == 'C' &&
			buf[3] == 'E')
		{
			//application name 
			int offset = (buf[85]<<8) | (buf[84]&0xff);
			int length = (buf[87]<<8) | (buf[86]&0xff);
			fseek (file_handle, offset, SEEK_SET);
			{
				char* name = new char [length+1];
				if (NULL != name)
				{
					fread (name, 1, length, file_handle);
					name[length] = '\0';

					std::cout << "App Name: " << name << std::endl;
					delete [] name;
				}
			}

			//provider
			offset = (buf[89]<<8) | (buf[88]&0xff);
			length = (buf[91]<<8) | (buf[90]&0xff);
			fseek (file_handle, offset, SEEK_SET);
			{
				char* name = new char [length+1];
				if (NULL != name)
				{
					fread (name, 1, length, file_handle);
					name[length] = '\0';

					std::cout << "Provider: " << name << std::endl;
					delete [] name;
				}
			}

			//string section
			offset = (buf[63]<<24) | (buf[62]<<16) | (buf[61]<<8) | (buf[60]&0xff);
			int num_of_entities = (buf[49]<<8) | (buf[48]&0xff);

			std::cout << "Strings Section..." << std::endl;
			
			fseek (file_handle, offset, SEEK_SET);
			char sect_hdr_buf[12];
			std::map< int, std::string > strings_entities;
			for (int i = 0; i < num_of_entities; ++ i)
			{
				fread (sect_hdr_buf, 1, 4, file_handle);
				int id = (sect_hdr_buf[1] << 8) | (sect_hdr_buf[0] & 0xff);
				length = (sect_hdr_buf[3] << 8) | (sect_hdr_buf[2] & 0xff);
				char* name = new char [length+1];
				if (NULL != name)
				{
					fread (name, 1, length, file_handle);
					name[length] = '\0';

					std:: cout << "  " << std::setfill('0') << std::setw(3) << id << ":" << name << std::endl;

					strings_entities[id] = name;
					delete [] name;
				}
			}

			//dirs
			offset = (buf[67]<<24) | (buf[66]<<16) | (buf[65]<<8) | (buf[64]&0xff);
			num_of_entities = (buf[51]<<8) | (buf[50]&0xff);

			std::cout << "Directories Section..." << std::endl;

			fseek (file_handle, offset, SEEK_SET);
			std::map< int, std::string > dirs_entities;
			for (int i = 0; i < num_of_entities; ++ i)
			{
				fread (sect_hdr_buf, 1, 4, file_handle);
				int id = (sect_hdr_buf[1] << 8) | (sect_hdr_buf[0] & 0xff);
				length = (sect_hdr_buf[3] << 8) | (sect_hdr_buf[2] & 0xff);
				char* name = new char [length+1];
				if (NULL != name)
				{
					fread (name, 1, length, file_handle);

					short * ids = (short*)name;
					std::string dir;
					while (*ids) 
					{
						dir += strings_entities[*ids];
						++ ids;

						if (ids) dir += '\\';
					}
					dir.replace (dir.find_last_of('\\'), dir.length(), "");
					if (dir.find("%CE") == 0)
					{
						dir.replace(0, 3, "");
						dir.replace(dir.find('%'), dir.length(), "");

						int num = strtol(dir.c_str(), NULL, 10);
						if (0 < num && num < sizeof (my_ce)/sizeof (my_ce[0]))
						{
							dir = my_ce[num];
						}
					}

					std:: cout << "  " << std::setfill('0') << std::setw(3) << id << ":" << dir << std::endl;

					dirs_entities[id] = dir;
					delete [] name;
				}
			}

			//files
			char dig[5];
			std::string dest;
			offset = (buf[71]<<24) | (buf[70]<<16) | (buf[69]<<8) | (buf[68]&0xff);
			num_of_entities = (buf[53]<<8) | (buf[52]&0xff);

			std::cout << "Files Section..." << std::endl;

			fseek (file_handle, offset, SEEK_SET);
			for (int i = 0; i < num_of_entities; ++ i)
			{
				fread (sect_hdr_buf, 1, 12, file_handle);
				short id   = (sect_hdr_buf[1] << 8) | (sect_hdr_buf[0] & 0xff);
				short dirid= (sect_hdr_buf[3] << 8) | (sect_hdr_buf[2] & 0xff);
				short unk  = (sect_hdr_buf[5] << 8) | (sect_hdr_buf[4] & 0xff);
				unsigned long flags = (sect_hdr_buf[9] << 24) | (sect_hdr_buf[8] << 16) | (sect_hdr_buf[7] << 8) | (sect_hdr_buf[6] & 0xff);
				length = (sect_hdr_buf[11] << 8) | (sect_hdr_buf[10] & 0xff);
				char* name = new char [length+1];
				
				if (NULL != name)
				{
					fread (name, 1, length, file_handle);
					name[length] = '\0';
					
					//processing
					sprintf (dig, ".%03d", id);
					if (get_fname (target, dig, fname))
					{
						dest = output + dirs_entities[dirid];

						//rename to the name to original path
						boost::filesystem::create_directories(dest);

						//output progess message
						dest += std::string("\\") + std::string(name);
						std::cout << "  Rename " << fname << " to " << dest << std::endl;

						//rename every file into the output path
						boost::filesystem::rename(fname, dest);
					}
					delete [] name;
				}
			}
			std::cout << std::endl << "Finished!" << std::endl;
		}
		else
		{
			std::cout << fname << ": not a Windows CE install cabinet header" << std::endl;

		}
		fclose (file_handle);
	}
	return 0;
}

