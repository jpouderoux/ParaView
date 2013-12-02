/*=========================================================================

  Program:   ParaView
  Module:    vtkPVTransposeTable.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVTransposeTable
//
// .SECTION Description

#ifndef __vtkPVTransposeTable_h
#define __vtkPVTransposeTable_h

#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports
#include "vtkTransposeTable.h"

class PVTransposeTableInternal;

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPVTransposeTable : public vtkTransposeTable
{
public:
  static vtkPVTransposeTable* New();
  vtkTypeMacro(vtkPVTransposeTable, vtkTransposeTable);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Interface for preparing selection of arrays in ParaView.
  void EnableAttributeArray(const char*);
  void ClearAttributeArrays();
  
protected:
  vtkPVTransposeTable();
  virtual ~vtkPVTransposeTable();

  int RequestData(vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector*);

  PVTransposeTableInternal* Internal;

private:
  vtkPVTransposeTable( const vtkPVTransposeTable& ); // Not implemented.
  void operator = ( const vtkPVTransposeTable& ); // Not implemented.
};

#endif // __vtkPVTransposeTable_h
