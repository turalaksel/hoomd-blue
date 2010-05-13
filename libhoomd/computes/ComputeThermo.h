/*
Highly Optimized Object-oriented Many-particle Dynamics -- Blue Edition
(HOOMD-blue) Open Source Software License Copyright 2008, 2009 Ames Laboratory
Iowa State University and The Regents of the University of Michigan All rights
reserved.

HOOMD-blue may contain modifications ("Contributions") provided, and to which
copyright is held, by various Contributors who have granted The Regents of the
University of Michigan the right to modify and/or distribute such Contributions.

Redistribution and use of HOOMD-blue, in source and binary forms, with or
without modification, are permitted, provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions, and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
list of conditions, and the following disclaimer in the documentation and/or
other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of HOOMD-blue's
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND/OR
ANY WARRANTIES THAT THIS SOFTWARE IS FREE OF INFRINGEMENT ARE DISCLAIMED.

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// $Id$
// $URL$
// Maintainer: joaander

#include <boost/shared_ptr.hpp>

#include "Compute.h"
#include "GPUArray.h"
#include "ComputeThermoTypes.h"
#include "ParticleGroup.h"

/*! \file ComputeThermo.h
    \brief Declares a class for computing thermodynamic quantities
*/

#ifndef __COMPUTE_THERMO_H__
#define __COMPUTE_THERMO_H__

//! Computes thermodynamic properties of a group of particles
/*! ComputeThermo calculates instantaneous thermodynamic properties and provides them for the logger.
    All computed values are stored in a GPUArray so that they can be accessed on the GPU without intermediate copies.
    Use the enum values in thermo_index to index the array and extract the properties of interest. Convenience functions
    are provided for accessing the values on the CPU. Certain properties, loke ndof and num_particles are always known
    and there is no need for them to be accessible via the GPUArray.

    Computed quantities available in the GPUArray:
     - temperature of the group
     - pressure (valid for the all group)
     - kinetic energy
     - potential energy

    Values available all the time
     - number of degrees of freedom (ndof)
     - number of particles in the group

    ndof is utilized in calculating the temperature from the kinetic energy. setNDOF() changes it to any value
    the user desires (the default is one!). In standard usage, the python interface queries the number of degrees
    of freedom from the integrators and sets that value for each ComputeThermo so that it is always correct.

    All quantities are made available for the logger. ComputerThermo can be given a suffix which it will append
    to each quantity provided to the logger. Typical usage is to provide _groupname as the suffix so that properties
    of different groups can be logged seperately (e.g. temperature_group1 and temperature_group2).

    \ingroup computes
*/
class ComputeThermo : public Compute
    {
    public:
        //! Constructs the compute
        ComputeThermo(boost::shared_ptr<SystemDefinition> sysdef,
                      boost::shared_ptr<ParticleGroup> group,
                      const std::string& suffix = std::string(""));
        
        //! Compute the temperature
        virtual void compute(unsigned int timestep);
        
        //! Change the number of degrees of freedom
        void setNDOF(unsigned int ndof);
        
        //! Get the number of degrees of freedom
        unsigned int getNDOF()
            {
            return m_ndof;
            }
            
        //! Returns the temperature last computed by compute()
        /*! \returns Instantaneous temperature of the system
        */
        Scalar getTemperature()
            {
            ArrayHandle<Scalar> h_properties(m_properties, access_location::host, access_mode::read);
            return h_properties.data[thermo_index::temperature];
            }
        
        //! Returns the pressure last computed by compute()
        /*! \returns Instantaneous pressure of the system
        */
        Scalar getPressure()
            {
            ArrayHandle<Scalar> h_properties(m_properties, access_location::host, access_mode::read);
            return h_properties.data[thermo_index::pressure];
            }
        
        //! Returns the kinetic energy last computed by compute()
        /*! \returns Instantaneous kinetic energy of the system
        */
        Scalar getKineticEnergy()
            {
            ArrayHandle<Scalar> h_properties(m_properties, access_location::host, access_mode::read);
            return h_properties.data[thermo_index::kinetic_energy];
            }
        
        //! Returns the potential energy last computed by compute()
        /*! \returns Instantaneous potential energy of the system
        */
        Scalar getPotentialEnergy()
            {
            ArrayHandle<Scalar> h_properties(m_properties, access_location::host, access_mode::read);
            return h_properties.data[thermo_index::potential_energy];
            }

        //! Get the gpu array of properties
        const GPUArray<Scalar>& getProperties()
            {
            return m_properties;
            }

        //! Returns a list of log quantities this compute calculates
        virtual std::vector< std::string > getProvidedLogQuantities();

        //! Calculates the requested log value and returns it
        virtual Scalar getLogValue(const std::string& quantity, unsigned int timestep);

    protected:
        boost::shared_ptr<ParticleGroup> m_group;     //!< Group to compute properties for
        GPUArray<Scalar> m_properties;  //!< Stores the computed properties
        unsigned int m_ndof;            //!< Stores the number of degrees of freedom in the system
        vector<string> m_logname_list;  //!< Cache all generated logged quantities names

        
        //! Does the actual computation
        virtual void computeProperties();
    };

//! Exports the ComputeThermo class to python
void export_ComputeThermo();

#endif
