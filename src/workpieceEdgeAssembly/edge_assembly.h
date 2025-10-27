#ifndef EDGE_ASSEMBLY_H
#define EDGE_ASSEMBLY_H

#include "src/jointDetection/contourProcess/joint_seam.h"

class EdgeAssembly
{
public:
    EdgeAssembly();

    void makeTestExample(JointSeam jointSeam);
};

#endif // EDGE_ASSEMBLY_H
