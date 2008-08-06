/*
Highly Optimized Object-Oriented Molecular Dynamics (HOOMD) Open
Source Software License
Copyright (c) 2008 Ames Laboratory Iowa State University
All rights reserved.

Redistribution and use of HOOMD, in source and binary forms, with or
without modification, are permitted, provided that the following
conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names HOOMD's
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND
CONTRIBUTORS ``AS IS''  AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS  BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

// $Id$
// $URL$

/*! \file MOL2DumpWriter.cc
	\brief Defines the MOL2DumpWriter class
*/

#ifdef USE_PYTHON
#include <boost/python.hpp>
using namespace boost::python;
#endif

#include <fstream>
#include <stdexcept>
#include <boost/shared_ptr.hpp>

#include "MOL2DumpWriter.h"
#include "BondData.h"

using namespace std;

/*! \param pdata Particle data to read when dumping files
	\param fname The file name to write the output to
*/
MOL2DumpWriter::MOL2DumpWriter(boost::shared_ptr<ParticleData> pdata, std::string fname)
	: Analyzer(pdata), m_fname(fname), m_written(false)
	{
	}

/*! \param timestep Current time step of the simulation
	Writes a snapshot of the current state of the ParticleData to a hoomd_xml file.
*/
void MOL2DumpWriter::analyze(unsigned int timestep)
	{
	if (m_written)
		{
		cout << "***Warning! MOL2 file " << m_fname << " already written, not overwriting." << endl;
		return;
		}
	
	// open the file for writing
	ofstream f(m_fname.c_str());
	
	if (!f.good())
		{
		cerr << endl << "***Error! Unable to open dump file for writing: " << m_fname << endl << endl;
		throw runtime_error("Error writting mol2 dump file");
		}

	// acquire the particle data
	ParticleDataArraysConst arrays = m_pdata->acquireReadOnly();
	
	// write the header
	f << "@<TRIPOS>MOLECULE" <<endl;
	f << "Generated by HOOMD" << endl;
	int num_bonds = 1;
	boost::shared_ptr<BondData> bond_data = m_pdata->getBondData();
	if (bond_data && bond_data->getNumBonds() > 0)
		num_bonds = bond_data->getNumBonds();
		
	f << m_pdata->getN() << " " << num_bonds << endl;
	f << "NO_CHARGES" << endl;

	f << "@<TRIPOS>ATOM" << endl;
	for (unsigned int j = 0; j < arrays.nparticles; j++)
		{
		// use the rtag data to output the particles in the order they were read in
		int i;
		i= arrays.rtag[j];
		
		// get the coordinates
		Scalar x = (arrays.x[i]);
		Scalar y = (arrays.y[i]);
		Scalar z = (arrays.z[i]);
		
		// get the type by name
		unsigned int type_id = arrays.type[i];
		string type_name = m_pdata->getNameByType(type_id);
		
		// this is intended to go to VMD, so limit the type name to 15 characters
		if (type_name.size() > 15)
			{
			cerr << endl << "Error! Type name <" << type_name << "> too long: please limit to 15 characters" << endl << endl;
			throw runtime_error("Error writting mol2 dump file");
			}
		
		f << j+1 << " " << type_name << " " << x << " " << y << " "<< z << " " << type_name << endl;

		if (!f.good())
			{
			cerr << endl << "***Error! Unexpected error writing MOL2 dump file" << endl << endl;
			throw runtime_error("Error writting mol2 dump file");
			}
		}

	// write a dummy bond since VMD doesn't like loading mol2 files without bonds
	// f << "@<TRIPOS>BOND" << endl;
	// f << "1 1 2 1" << endl;
	
	f << "@<TRIPOS>BOND" << endl;
	if (bond_data && bond_data->getNumBonds() > 0)
		{
		for (unsigned int i = 0; i < bond_data->getNumBonds(); i++)
			{
			Bond b = bond_data->getBond(i);
			f << i+1 << " " << b.a+1 << " " << b.b+1 << " 1" << endl;
			}
		}
	else
		{
		f << "1 1 2 1" << endl;
		}



	if (!f.good())
		{
		cerr << endl << "***Error! Unexpected error writing HOOMD dump file" << endl << endl;
		throw runtime_error("Error writting mol2 dump file");
		}

	f.close();
	m_pdata->release();
	
	m_written = true;
	}

#ifdef USE_PYTHON
void export_MOL2DumpWriter()
	{
	class_<MOL2DumpWriter, boost::shared_ptr<MOL2DumpWriter>, bases<Analyzer>, boost::noncopyable>
		("MOL2DumpWriter", init< boost::shared_ptr<ParticleData>, std::string >())
		.def( init< boost::shared_ptr<ParticleData>, std::string >() )
		;
	// no .defs, everything is inherited
	}
#endif
	
