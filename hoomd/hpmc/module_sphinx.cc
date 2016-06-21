// Copyright (c) 2009-2016 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

// Include the defined classes that are to be exported to python
#include "IntegratorHPMC.h"
#include "IntegratorHPMCMono.h"
#include "IntegratorHPMCMonoImplicit.h"
#include "ComputeFreeVolume.h"

#include "ShapeSphere.h"
#include "ShapeConvexPolygon.h"
#include "ShapePolyhedron.h"
#include "ShapeConvexPolyhedron.h"
#include "ShapeSpheropolyhedron.h"
#include "ShapeSpheropolygon.h"
#include "ShapeSimplePolygon.h"
#include "ShapeEllipsoid.h"
#include "ShapeFacetedSphere.h"
#include "ShapeSphinx.h"
#include "AnalyzerSDF.h"
#include "ShapeUnion.h"

#include "ExternalField.h"
#include "ExternalFieldWall.h"
#include "ExternalFieldLattice.h"
#include "ExternalFieldComposite.h"

#include "UpdaterExternalFieldWall.h"
#include "UpdaterRemoveDrift.h"
#include "UpdaterMuVT.h"
#include "UpdaterMuVTImplicit.h"

#ifdef ENABLE_CUDA
#include "IntegratorHPMCMonoGPU.h"
#include "IntegratorHPMCMonoImplicitGPU.h"
#include "ComputeFreeVolumeGPU.h"
#endif

// Include boost.python to do the exporting
#include <boost/python.hpp>

using namespace boost::python;
using namespace hpmc;

using namespace hpmc::detail;

namespace hpmc
{

//! Export the base HPMCMono integrators
void export_sphinx()
    {
    export_IntegratorHPMCMono< ShapeSphinx >("IntegratorHPMCMonoSphere");
    export_IntegratorHPMCMonoImplicit< ShapeSphinx >("IntegratorHPMCMonoImplicitSphere");
    export_ComputeFreeVolume< ShapeSphinx >("ComputeFreeVolumeSphere");
    export_AnalyzerSDF< ShapeSphinx >("AnalyzerSDFSphere");
    // export_UpdaterMuVT< ShapeSphinx >("UpdaterMuVTSphere");
    // export_UpdaterMuVTImplicit< ShapeSphinx >("UpdaterMuVTImplicitSphere");

    export_ExternalFieldInterface<ShapeSphinx>("ExternalFieldSphere");
    export_LatticeField<ShapeSphinx>("ExternalFieldLatticeSphere");
    export_ExternalFieldComposite<ShapeSphinx>("ExternalFieldCompositeSphere");
    export_RemoveDriftUpdater<ShapeSphinx>("RemoveDriftUpdaterSphere");
    export_ExternalFieldWall<ShapeSphinx>("WallSphere");
    export_UpdaterExternalFieldWall<ShapeSphinx>("UpdaterExternalFieldWallSphere");

    #ifdef ENABLE_CUDA
    #ifdef ENABLE_SPHINX_GPU

    export_IntegratorHPMCMonoGPU< ShapeSphinx >("IntegratorHPMCMonoGPUSphere");
    export_IntegratorHPMCMonoImplicitGPU< ShapeSphinx >("IntegratorHPMCMonoImplicitGPUSphere");
    export_ComputeFreeVolumeGPU< ShapeSphinx >("ComputeFreeVolumeGPUSphere");

    #endif
    #endif
    }

}
