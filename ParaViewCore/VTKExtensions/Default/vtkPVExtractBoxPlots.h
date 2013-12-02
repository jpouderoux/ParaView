/*=========================================================================

  Program:   ParaView
  Module:    vtkPVExtractBoxPlots.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVExtractBoxPlots
//
// .SECTION Description

#ifndef __vtkPVExtractBoxPlots_h
#define __vtkPVExtractBoxPlots_h

#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports
#include "vtkTransposeTable.h"

class PVExtractBoxPlotsInternal;

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPVExtractBoxPlots : public vtkTransposeTable
{
public:
  static vtkPVExtractBoxPlots* New();
  vtkTypeMacro(vtkPVExtractBoxPlots, vtkTransposeTable);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Interface for preparing selection of arrays in ParaView.
  void EnableAttributeArray(const char*);
  void ClearAttributeArrays();
  
  vtkGetMacro(TransposeTable, bool);
  vtkSetMacro(TransposeTable, bool);
  vtkBooleanMacro(TransposeTable, bool);
  
  vtkGetMacro(Sigma, double);
  vtkSetMacro(Sigma, double);
  
protected:
  vtkPVExtractBoxPlots();
  virtual ~vtkPVExtractBoxPlots();

  int RequestData(vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector*);

  PVExtractBoxPlotsInternal* Internal;
  
  bool TransposeTable;
  double Sigma;

private:
  vtkPVExtractBoxPlots( const vtkPVExtractBoxPlots& ); // Not implemented.
  void operator = ( const vtkPVExtractBoxPlots& ); // Not implemented.
};

#endif // __vtkPVExtractBoxPlots_h
