/*=========================================================================

  Program:   ParaView
  Module:    vtkSciVizHighestDensityRegionsStats.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSciVizHighestDensityRegionsStats
// .SECTION Description

#ifndef __vtkSciVizHighestDensityRegionsStats_h
#define __vtkSciVizHighestDensityRegionsStats_h

#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports
#include "vtkSciVizStatistics.h"

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkSciVizHighestDensityRegionsStats : public vtkSciVizStatistics
{
public:
  static vtkSciVizHighestDensityRegionsStats* New();
  vtkTypeMacro(vtkSciVizHighestDensityRegionsStats,vtkSciVizStatistics);
  virtual void PrintSelf( ostream& os, vtkIndent indent );

  // Description:
  // Smoothing kernel size of the HDR. Default is 1
  vtkGetMacro(Sigma, double);
  vtkSetMacro(Sigma, double);

protected:
  vtkSciVizHighestDensityRegionsStats();
  virtual ~vtkSciVizHighestDensityRegionsStats();

  virtual int LearnAndDerive( vtkMultiBlockDataSet* model, vtkTable* inData );
  virtual int AssessData( vtkTable* observations, vtkDataObject* dataset, vtkMultiBlockDataSet* model );

  double Sigma;

private:
  vtkSciVizHighestDensityRegionsStats( const vtkSciVizHighestDensityRegionsStats& ); // Not implemented.
  void operator = ( const vtkSciVizHighestDensityRegionsStats& ); // Not implemented.
};

#endif // __vtkSciVizHighestDensityRegionsStats_h
