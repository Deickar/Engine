/*
http://stackoverflow.com/questions/86582/singleton-how-should-it-be-used
http://stackoverflow.com/questions/10892167/proper-implementation-of-global-configuration

 * C++ Singleton
 * Limitation: Single Threaded Design
 * See: http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
 *      For problems associated with locking in multi threaded applications
 *
 * Limitation:
 * If you use this Singleton (A) within a destructor of another Singleton (B)
 * This Singleton (A) must be fully constructed before the constructor of (B)
 * is called.
 */

#pragma once
#include "SchladetschEvents.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

using namespace std;
using namespace Schladetsch::Events;
using namespace libconfig;

static const std::string filepath = "settings.cfg";

class SettingsManager
{
	private:
		// Private Constructor
		SettingsManager();

		//Stop the compiler from generating methods of copying the object
		SettingsManager(SettingsManager const& copy);            // Not Implemented
		SettingsManager& operator=(SettingsManager const& copy); // Not Implemented

	public:
		static SettingsManager& GetInstance()
		{
			// The only instance
			// Guaranteed to be lazy initialized
			// Guaranteed that it will be destroyed correctly
			static SettingsManager instance;
			return instance;
		}

		//As separate function because we also want to trigger the re-load event
		void ReloadSettings();
		//Config& GetConfigFile() { return cfg; }
		//void SaveSettings(std::string filePath);

		/*
		Used like this:

		Foo foo;
		event.Add(foo, &Foo::Method);
		*/
		//Config& GetConfigRoot() { return cfg; }
		Event<Config*>* GetEvent() { return &loadEvent; }

	private:
		//Empty template, because we just want to call functions on rebuild
		Event<Config*> loadEvent;
		Config cfg;
};


 /* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2010  Mark A Lindner

   This file is part of libconfig.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

/*
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

using namespace std;
using namespace libconfig;

// This example reads the configuration file 'example.cfg' and displays
// some of its contents.

int main(int argc, char **argv)
{
  Config cfg;

  // Read the file. If there is an error, report it and exit.
  try
  {
	cfg.readFile("example.cfg");
  }
  catch(const FileIOException &fioex)
  {
	std::cerr << "I/O error while reading file." << std::endl;
	return(EXIT_FAILURE);
  }
  catch(const ParseException &pex)
  {
	std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			  << " - " << pex.getError() << std::endl;
	return(EXIT_FAILURE);
  }

  // Get the store name.
  try
  {
	string name = cfg.lookup("name");
	cout << "Store name: " << name << endl << endl;
  }
  catch(const SettingNotFoundException &nfex)
  {
	cerr << "No 'name' setting in configuration file." << endl;
  }

  const Setting& root = cfg.getRoot();

  // Output a list of all books in the inventory.
  try
  {
	const Setting &books = root["inventory"]["books"];
	int count = books.getLength();

	cout << setw(30) << left << "TITLE" << "  "
		 << setw(30) << left << "AUTHOR" << "   "
		 << setw(6) << left << "PRICE" << "  "
		 << "QTY"
		 << endl;

	for(int i = 0; i < count; ++i)
	{
	  const Setting &book = books[i];

	  // Only output the record if all of the expected fields are present.
	  string title, author;
	  double price;
	  int qty;

	  if(!(book.lookupValue("title", title)
		   && book.lookupValue("author", author)
		   && book.lookupValue("price", price)
		   && book.lookupValue("qty", qty)))
		continue;

	  cout << setw(30) << left << title << "  "
		   << setw(30) << left << author << "  "
		   << '$' << setw(6) << right << price << "  "
		   << qty
		   << endl;
	}
	cout << endl;
  }
  catch(const SettingNotFoundException &nfex)
  {
	// Ignore.
  }

  // Output a list of all books in the inventory.
  try
  {
	const Setting &movies = root["inventory"]["movies"];
	int count = movies.getLength();

	cout << setw(30) << left << "TITLE" << "  "
		 << setw(10) << left << "MEDIA" << "   "
		 << setw(6) << left << "PRICE" << "  "
		 << "QTY"
		 << endl;

	for(int i = 0; i < count; ++i)
	{
	  const Setting &movie = movies[i];

	  // Only output the record if all of the expected fields are present.
	  string title, media;
	  double price;
	  int qty;

	  if(!(movie.lookupValue("title", title)
		   && movie.lookupValue("media", media)
		   && movie.lookupValue("price", price)
		   && movie.lookupValue("qty", qty)))
		continue;

	  cout << setw(30) << left << title << "  "
		   << setw(10) << left << media << "  "
		   << '$' << setw(6) << right << price << "  "
		   << qty
		   << endl;
	}
	cout << endl;
  }
  catch(const SettingNotFoundException &nfex)
  {
	// Ignore.
  }

  return(EXIT_SUCCESS);
}

// eof

 */