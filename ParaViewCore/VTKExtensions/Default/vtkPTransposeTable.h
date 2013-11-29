/*=========================================================================

  Program:   ParaView
  Module:    vtkPTransposeTable.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPTransposeTable
//
// .SECTION Description

#ifndef __vtkPTransposeTable_h
#define __vtkPTransposeTable_h

#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports
#include "vtkTransposeTable.h"

class PTransposeTableInternal;

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPTransposeTable : public vtkTransposeTable
{
public:
  static vtkPTransposeTable* New();
  vtkTypeMacro(vtkPTransposeTable, vtkTransposeTable);
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set/get the type of field attribute (cell, point, field)
  vtkGetMacro(AttributeMode,int);
  vtkSetMacro(AttributeMode,int);
  
  // Description:
  // Return the number of columns available for the current table.
  int GetNumberOfAttributeArrays();

  // Description:
  // Get the name of the \a n-th array ffor the current table
  const char* GetAttributeArrayName( int n );

  // Description:
  // Get the status of the specified array (i.e., whether or not it is a column of interest).
  int GetAttributeArrayStatus( const char* arrName );

  // Description:
  // An alternate interface for preparing a selection of arrays in ParaView.
  void EnableAttributeArray( const char* arrName );
  void ClearAttributeArrays();
  
protected:
  vtkPTransposeTable();
  virtual ~vtkPTransposeTable();

  int RequestData(vtkInformation*,
    vtkInformationVector**,
    vtkInformationVector*);

  PTransposeTableInternal* Internal;
  int AttributeMode;

private:
  vtkPTransposeTable( const vtkPTransposeTable& ); // Not implemented.
  void operator = ( const vtkPTransposeTable& ); // Not implemented.
};

#endif // __vtkPTransposeTable_h
