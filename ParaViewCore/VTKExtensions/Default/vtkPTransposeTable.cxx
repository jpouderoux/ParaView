#include "vtkPTransposeTable.h"

#include "vtkDataSetAttributes.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"

#include <set>
#include <string>

class PTransposeTableInternal
{
public:
  bool Clear() { if (this->Columns.empty()) return false; this->Columns.clear(); return true; }
  bool Has(const std::string& v) { return this->Columns.find(v) != this->Columns.end(); }
  bool Set(const std::string& v) { if (this->Has(v)) return false; this->Columns.insert(v); return true;}
protected:
  std::set<std::string> Columns;
};

vtkStandardNewMacro(vtkPTransposeTable);

vtkPTransposeTable::vtkPTransposeTable()
{
  this->Internal = new PTransposeTableInternal();
  this->AttributeMode = vtkDataObject::ROW;
}

vtkPTransposeTable::~vtkPTransposeTable()
{
  delete this->Internal;
}

int vtkPTransposeTable::GetNumberOfAttributeArrays()
{
  vtkTable* table = vtkTable::SafeDownCast(this->GetInputDataObject( 0, 0 )); // First input is always the leader
  if ( ! table )
    {
    return 0;
    }

  return table->GetNumberOfColumns();
}

const char* vtkPTransposeTable::GetAttributeArrayName( int n )
{
  vtkTable* table = vtkTable::SafeDownCast(this->GetInputDataObject( 0, 0 )); // First input is always the leader
  if ( ! table )
    {
    return 0;
    }

  int numCols = table->GetNumberOfColumns();
  if ( n < 0 || n > numCols )
    {
    return 0;
    }

  vtkAbstractArray* arr = table->GetColumn( n );
  if ( ! arr )
    {
    return 0;
    }

  return arr->GetName();
}

int vtkPTransposeTable::GetAttributeArrayStatus( const char* arrName )
{  
  return this->Internal->Has( std::string(arrName) ) ? 1 : 0;
}

void vtkPTransposeTable::EnableAttributeArray( const char* arrName )
{
  if ( arrName )
    {
    if ( this->Internal->Set( arrName ) )
      {
      this->Modified();
      }
    }
}

void vtkPTransposeTable::ClearAttributeArrays()
{
  if ( this->Internal->Clear() )
    {
    this->Modified();
    }
}

void vtkPTransposeTable::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
int vtkPTransposeTable::RequestData(
  vtkInformation* info,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // vtkTable* inTable = vtkTable::GetData(inputVector[0]);
  // vtkTable* outTable = vtkTable::GetData(outputVector, 0);
  
  // if (inTable->GetNumberOfColumns() == 0)
    // {
    // vtkErrorMacro(<< 
      // "vtkTransposeTable requires vtkTable containing at least one column.");
    // return 0;
    // }

  
  return this->Superclass::RequestData(info, inputVector, outputVector);
}
