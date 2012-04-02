/*
Highly Optimized Object-oriented Many-particle Dynamics -- Blue Edition
(HOOMD-blue) Open Source Software License Copyright 2008-2011 Ames Laboratory
Iowa State University and The Regents of the University of Michigan All rights
reserved.

HOOMD-blue may contain modifications ("Contributions") provided, and to which
copyright is held, by various Contributors who have granted The Regents of the
University of Michigan the right to modify and/or distribute such Contributions.

You may redistribute, use, and create derivate works of HOOMD-blue, in source
and binary forms, provided you abide by the following conditions:

* Redistributions of source code must retain the above copyright notice, this
list of conditions, and the following disclaimer both in the code and
prominently in any materials provided with the distribution.

* Redistributions in binary form must reproduce the above copyright notice, this
list of conditions, and the following disclaimer in the documentation and/or
other materials provided with the distribution.

* All publications and presentations based on HOOMD-blue, including any reports
or published results obtained, in whole or in part, with HOOMD-blue, will
acknowledge its use according to the terms posted at the time of submission on:
http://codeblue.umich.edu/hoomd-blue/citations.html

* Any electronic documents citing HOOMD-Blue will link to the HOOMD-Blue website:
http://codeblue.umich.edu/hoomd-blue/

* Apart from the above required attributions, neither the name of the copyright
holder nor the names of HOOMD-blue's contributors may be used to endorse or
promote products derived from this software without specific prior written
permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND/OR ANY
WARRANTIES THAT THIS SOFTWARE IS FREE OF INFRINGEMENT ARE DISCLAIMED.

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Maintainer: joaander

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4267 4244 )
#endif

#include "ParticleGroup.h"

#include <boost/python.hpp>
#include <boost/bind.hpp>
using namespace boost::python;
using namespace boost;

#include <algorithm>
#include <iostream>
using namespace std;

/*! \file ParticleGroup.cc
    \brief Defines the ParticleGroup and related classes
*/

//////////////////////////////////////////////////////////////////////////////
// ParticleSelector

/*! \param sysdef System the particles are to be selected from
*/
ParticleSelector::ParticleSelector(boost::shared_ptr<SystemDefinition> sysdef)
    : m_sysdef(sysdef), m_pdata(sysdef->getParticleData())
    {
    assert(m_sysdef);
    assert(m_pdata);
    }

/*! \param tag Tag of the particle to check
    \returns true if the particle is selected
    \returns false if it is not
*/
bool ParticleSelector::isSelected(unsigned int tag) const
    {
    // base class doesn't do anything useful
    return false;
    }

//////////////////////////////////////////////////////////////////////////////
// ParticleSelectorTag

/*! \param sysdef System the particles are to be selected from
    \param tag_min Minimum tag to select (inclusive)
    \param tag_max Maximum tag to select (inclusive)
*/
ParticleSelectorTag::ParticleSelectorTag(boost::shared_ptr<SystemDefinition> sysdef,
                                         unsigned int tag_min,
                                         unsigned int tag_max)
    : ParticleSelector(sysdef), m_tag_min(tag_min), m_tag_max(tag_max)
    {
    // make a quick check on the sanity of the input data
    if (m_tag_max < m_tag_min)
        cout << "***Warning! max < min specified when selecting particle tags" << endl;
    
    if (m_tag_max >= m_pdata->getN())
        {
        cerr << endl << "***Error! Cannot select particles with tags larger than the number of particles " 
             << endl << endl;
        throw runtime_error("Error selecting particles");
        }
    }

/*! \param tag Tag of the particle to check
    \returns true if \a m_tag_min <= \a tag <= \a m_tag_max
*/
bool ParticleSelectorTag::isSelected(unsigned int tag) const
    {
    assert(tag < m_pdata->getN());
    return (m_tag_min <= tag && tag <= m_tag_max);
    }

//////////////////////////////////////////////////////////////////////////////
// ParticleSelectorType

/*! \param sysdef System the particles are to be selected from
    \param typ_min Minimum type id to select (inclusive)
    \param typ_max Maximum type id to select (inclusive)
*/
ParticleSelectorType::ParticleSelectorType(boost::shared_ptr<SystemDefinition> sysdef,
                                           unsigned int typ_min,
                                           unsigned int typ_max)
    : ParticleSelector(sysdef), m_typ_min(typ_min), m_typ_max(typ_max)
    {
    // make a quick check on the sanity of the input data
    if (m_typ_max < m_typ_min)
        cout << "***Warning! max < min specified when selecting particle types" << endl;
    
    if (m_typ_max >= m_pdata->getNTypes())
        cout << "***Warning! Requesting for the selection of a non-existant particle type" << endl;
    }

/*! \param tag Tag of the particle to check
    \returns true if the type of particle \a tag is in the inclusive range [ \a m_typ_min, \a m_typ_max ]
*/
bool ParticleSelectorType::isSelected(unsigned int tag) const
    {
    assert(tag < m_pdata->getN());
    unsigned int typ = m_pdata->getType(tag);
    
    // see if it matches the criteria
    bool result = (m_typ_min <= typ && typ <= m_typ_max);
    
    return result;
    }

//////////////////////////////////////////////////////////////////////////////
// ParticleSelectorRigid

/*! \param sysdef System the particles are to be selected from
    \param rigid true selects particles that are in rigid bodies, false selects particles that are not part of a body
*/
ParticleSelectorRigid::ParticleSelectorRigid(boost::shared_ptr<SystemDefinition> sysdef,
                                             bool rigid)
    : ParticleSelector(sysdef), m_rigid(rigid)
    {
    }

/*! \param tag Tag of the particle to check
    \returns true if the type of particle \a tag meets the rigid critera selected
*/
bool ParticleSelectorRigid::isSelected(unsigned int tag) const
    {
    assert(tag < m_pdata->getN());
    
    // get body id of current particle tag
    unsigned int body = m_pdata->getBody(tag);
    
    // see if it matches the criteria
    bool result = false;
    if (m_rigid && body != NO_BODY)
        result = true;
    if (!m_rigid && body == NO_BODY)
        result = true;
    
    return result;
    }

//////////////////////////////////////////////////////////////////////////////
// ParticleSelectorCuboid

ParticleSelectorCuboid::ParticleSelectorCuboid(boost::shared_ptr<SystemDefinition> sysdef, Scalar3 min, Scalar3 max)
    :ParticleSelector(sysdef), m_min(min), m_max(max)
    {
    // make a quick check on the sanity of the input data
    if (m_min.x >= m_max.x || m_min.y >= m_max.y || m_min.z >= m_max.z)
        cout << "***Warning! max < min specified when selecting particle in a cuboid" << endl;
    }

/*! \param tag Tag of the particle to check
    \returns true if the type of particle \a tag is in the cuboid
    
    Evaluation is performed by \a m_min.x <= x < \a m_max.x so that multiple cuboids stacked next to each other
    do not have overlapping sets of particles.
*/
bool ParticleSelectorCuboid::isSelected(unsigned int tag) const
    {
    assert(tag < m_pdata->getN());

    // identify the index of the current particle tag
    Scalar3 pos = m_pdata->getPosition(tag);
    
    // see if it matches the criteria
    bool result = (m_min.x <= pos.x && pos.x < m_max.x &&
                   m_min.y <= pos.y && pos.y < m_max.y &&
                   m_min.z <= pos.z && pos.z < m_max.z);
    
    return result;
    }

//////////////////////////////////////////////////////////////////////////////
// ParticleGroup

/*! \param sysdef System definition to build the group from
    \param selector ParticleSelector used to choose the group members

    Particles where criteria falls within the range [min,max] (inclusive) are added to the group.
*/
ParticleGroup::ParticleGroup(boost::shared_ptr<SystemDefinition> sysdef, boost::shared_ptr<ParticleSelector> selector)
    : m_sysdef(sysdef),
      m_pdata(sysdef->getParticleData()),
      m_is_member(m_pdata->getN())
    {
    // assign all of the particles that belong to the group
    // for each particle in the data
    for (unsigned int tag = 0; tag < m_pdata->getN(); tag++)
        {
        // add the tag to the list if it matches the selection
        if (selector->isSelected(tag))
            m_member_tags.push_back(tag);
        }
    
    GPUArray<unsigned int> member_idx(m_member_tags.size(), m_pdata->getExecConf());
    m_member_idx.swap(member_idx);
    
    // now that the tag list is completely set up and all memory is allocated, rebuild the index list
    rebuildIndexList();
    
    // connect the rebuildIndexList method to be called whenever the particles are sorted
    m_sort_connection = m_pdata->connectParticleSort(bind(&ParticleGroup::rebuildIndexList, this));
    }

/*! \param sysdef System definition to build the group from
    \param member_tags List of particle tags that belong to the group
    
    All particles specified in \a member_tags will be added to the group.
*/
ParticleGroup::ParticleGroup(boost::shared_ptr<SystemDefinition> sysdef, const std::vector<unsigned int>& member_tags)
    : m_sysdef(sysdef),
      m_pdata(sysdef->getParticleData()),
      m_is_member(m_pdata->getN()),
      m_member_tags(member_tags)
    {
    // let's make absolutely sure that the tag order given from outside is sorted
    sort(m_member_tags.begin(), m_member_tags.end());
    
    GPUArray<unsigned int> member_idx(m_member_tags.size(), m_pdata->getExecConf());
    m_member_idx.swap(member_idx);
    
    // now that the tag list is completely set up and all memory is allocated, rebuild the index list
    rebuildIndexList();
    
    // connect the rebuildIndexList method to be called whenever the particles are sorted
    m_sort_connection = m_pdata->connectParticleSort(bind(&ParticleGroup::rebuildIndexList, this));
    }

ParticleGroup::~ParticleGroup()
    {
    // disconnect the sort connection, but only if there was a particle data to connect it to in the first place
    if (m_pdata)
        m_sort_connection.disconnect();
    }

/*! \returns Total mass of all particles in the group
    \note This method aquires the ParticleData internally
*/
Scalar ParticleGroup::getTotalMass() const
    {
    // grab the particle data
    ArrayHandle< Scalar4 > h_vel(m_pdata->getVelocities(), access_location::host, access_mode::read);

    // loop  through all indices in the group and total the mass
    Scalar total_mass = 0.0;
    for (unsigned int i = 0; i < getNumMembers(); i++)
        {
        unsigned int idx = getMemberIndex(i);
        total_mass += h_vel.data[idx].w;
        }
    return total_mass;
    }
    
/*! \returns The center of mass of the group, in unwrapped coordinates
    \note This method aquires the ParticleData internally
*/
Scalar3 ParticleGroup::getCenterOfMass() const
    {
    // grab the particle data
    ArrayHandle< Scalar4 > h_vel(m_pdata->getVelocities(), access_location::host, access_mode::read);
    ArrayHandle< Scalar4 > h_pos(m_pdata->getPositions(), access_location::host, access_mode::read);
    ArrayHandle< int3 > h_image(m_pdata->getImages(), access_location::host, access_mode::read);
    
    // grab the box dimensions
    BoxDim box = m_pdata->getBox();
    Scalar3 L = box.getL();
    
    // loop  through all indices in the group and compute the weighted average of the positions
    Scalar total_mass = 0.0;
    Scalar3 center_of_mass = make_scalar3(Scalar(0.0), Scalar(0.0), Scalar(0.0));
    for (unsigned int i = 0; i < getNumMembers(); i++)
        {
        unsigned int idx = getMemberIndex(i);
        Scalar mass = h_vel.data[idx].w;
        total_mass += mass;
        center_of_mass.x += mass * (h_pos.data[idx].x + Scalar(h_image.data[idx].x) * L.x);
        center_of_mass.y += mass * (h_pos.data[idx].y + Scalar(h_image.data[idx].y) * L.y);
        center_of_mass.z += mass * (h_pos.data[idx].z + Scalar(h_image.data[idx].z) * L.z);
        }
    center_of_mass.x /= total_mass;
    center_of_mass.y /= total_mass;
    center_of_mass.z /= total_mass;

    return center_of_mass;
    }

/*! \param a First particle group
    \param b Second particle group

    \returns A shared pointer to a newly created particle group that contains all the elements present in \a a and
    \a b
*/
boost::shared_ptr<ParticleGroup> ParticleGroup::groupUnion(boost::shared_ptr<ParticleGroup> a,
                                                           boost::shared_ptr<ParticleGroup> b)
    {
    // vector to store the new list of tags
    vector<unsigned int> member_tags;
    
    // make the union
    insert_iterator< vector<unsigned int> > ii(member_tags, member_tags.begin());
    set_union(a->m_member_tags.begin(), a->m_member_tags.end(), b->m_member_tags.begin(), b->m_member_tags.end(), ii);
    
    // create the new particle group
    boost::shared_ptr<ParticleGroup> new_group(new ParticleGroup(a->m_sysdef, member_tags));
    
    // return the newly created group
    return new_group;
    }

/*! \param a First particle group
    \param b Second particle group

    \returns A shared pointer to a newly created particle group that contains only the elements present in both \a a and
    \a b
*/
boost::shared_ptr<ParticleGroup> ParticleGroup::groupIntersection(boost::shared_ptr<ParticleGroup> a,
                                                                  boost::shared_ptr<ParticleGroup> b)
    {
    // vector to store the new list of tags
    vector<unsigned int> member_tags;
    
    // make the intersection
    insert_iterator< vector<unsigned int> > ii(member_tags, member_tags.begin());
    set_intersection(a->m_member_tags.begin(), a->m_member_tags.end(), b->m_member_tags.begin(), b->m_member_tags.end(), ii);
    
    // create the new particle group
    boost::shared_ptr<ParticleGroup> new_group(new ParticleGroup(a->m_sysdef, member_tags));
    
    // return the newly created group
    return new_group;
    }

/*! \param a First particle group
    \param b Second particle group

    \returns A shared pointer to a newly created particle group that contains only the elements present in \a a, and
    not any present in \a b
*/
boost::shared_ptr<ParticleGroup> ParticleGroup::groupDifference(boost::shared_ptr<ParticleGroup> a,
                                                                boost::shared_ptr<ParticleGroup> b)
    {
    // vector to store the new list of tags
    vector<unsigned int> member_tags;
    
    // make the difference
    insert_iterator< vector<unsigned int> > ii(member_tags, member_tags.begin());
    set_difference(a->m_member_tags.begin(), a->m_member_tags.end(), b->m_member_tags.begin(), b->m_member_tags.end(), ii);
    
    // create the new particle group
    boost::shared_ptr<ParticleGroup> new_group(new ParticleGroup(a->m_sysdef, member_tags));
    
    // return the newly created group
    return new_group;
    }

/*! \pre m_member_tags has been filled out, listing all particle tags in the group
    \pre memory has been allocated for m_is_member and m_member_idx
    \post m_is_member is updated so that it reflects the current indices of the particles in the group
    \post m_member_idx is updated listing all particle indices belonging to the group, in index order
*/
void ParticleGroup::rebuildIndexList()
    {
    // start by rebuilding the bitset of member indices in the group
    // it needs to be cleared first
    m_is_member.reset();
    
    // then loop through every particle in the group and set its bit
    {
    ArrayHandle< unsigned int > h_rtag(m_pdata->getRTags(), access_location::host, access_mode::read);

    for (unsigned int member_idx = 0; member_idx < m_member_tags.size(); member_idx++)
        {
        unsigned int idx = h_rtag.data[m_member_tags[member_idx]];
        m_is_member[idx] = true;
        }
    }

    // then loop through the bitset and add indices to the index list
    ArrayHandle<unsigned int> h_handle(m_member_idx, access_location::host, access_mode::readwrite);
    unsigned int cur_member = 0;
    unsigned int nparticles = m_pdata->getN();
    for (unsigned int idx = 0; idx < nparticles; idx++)
        {
        if (isMember(idx))
            {
            h_handle.data[cur_member] = idx;
            cur_member++;
            }
        }
    
    // sanity check, the number of indices added to m_member_idx must be the same as the number of members in the group
    assert(cur_member == m_member_tags.size());
    }

void export_ParticleGroup()
    {
    class_<ParticleGroup, boost::shared_ptr<ParticleGroup>, boost::noncopyable>
            ("ParticleGroup", init< boost::shared_ptr<SystemDefinition>, boost::shared_ptr<ParticleSelector> >())
            .def(init<boost::shared_ptr<SystemDefinition>, const std::vector<unsigned int>& >())
            .def(init<>())
            .def("getNumMembers", &ParticleGroup::getNumMembers)
            .def("getMemberTag", &ParticleGroup::getMemberTag)
            .def("getTotalMass", &ParticleGroup::getTotalMass)
            .def("getCenterOfMass", &ParticleGroup::getCenterOfMass)
            .def("groupUnion", &ParticleGroup::groupUnion)
            .def("groupIntersection", &ParticleGroup::groupIntersection)
            .def("groupDifference", &ParticleGroup::groupDifference)
            ;
    
    class_<ParticleSelector, boost::shared_ptr<ParticleSelector>, boost::noncopyable>
            ("ParticleSelector", init< boost::shared_ptr<SystemDefinition> >())
            .def("isSelected", &ParticleSelector::isSelected)
            ;
    
    class_<ParticleSelectorTag, boost::shared_ptr<ParticleSelectorTag>, bases<ParticleSelector>, boost::noncopyable>
        ("ParticleSelectorTag", init< boost::shared_ptr<SystemDefinition>, unsigned int, unsigned int >())
        ;
        
    class_<ParticleSelectorType, boost::shared_ptr<ParticleSelectorType>, bases<ParticleSelector>, boost::noncopyable>
        ("ParticleSelectorType", init< boost::shared_ptr<SystemDefinition>, unsigned int, unsigned int >())
        ;

    class_<ParticleSelectorRigid, boost::shared_ptr<ParticleSelectorRigid>, bases<ParticleSelector>, boost::noncopyable>
        ("ParticleSelectorRigid", init< boost::shared_ptr<SystemDefinition>, bool >())
        ;    

    class_<ParticleSelectorCuboid, boost::shared_ptr<ParticleSelectorCuboid>, bases<ParticleSelector>, boost::noncopyable>
        ("ParticleSelectorCuboid", init< boost::shared_ptr<SystemDefinition>, Scalar3, Scalar3 >())
        ;
    }

#ifdef WIN32
#pragma warning( pop )
#endif

