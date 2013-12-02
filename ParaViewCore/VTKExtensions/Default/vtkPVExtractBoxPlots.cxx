#include "vtkPVExtractBoxPlots.h"

#include "vtkAbstractArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkTable.h"
#include "vtkTransposeTable.h"
#include "vtkPCAStatistics.h"
#include "vtkPSciVizPCAStats.h"
#include "vtkHighestDensityRegionsStatistics.h"

#include <set>
#include <string>

//----------------------------------------------------------------------------
// Internal class that holds selected columns
class PVExtractBoxPlotsInternal
{
public:
  bool Clear()
  { 
    if (this->Columns.empty())
      {
      return false;
      }
    this->Columns.clear();
    return true; 
  }

  bool Has(const std::string& v)
  { 
    return this->Columns.find(v) != this->Columns.end();
  }

  bool Set(const std::string& v)
  { 
  if (this->Has(v))
    {
    return false;
    }
  this->Columns.insert(v);
  return true;
  }

  std::set<std::string> Columns;
};

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVExtractBoxPlots);

//----------------------------------------------------------------------------
vtkPVExtractBoxPlots::vtkPVExtractBoxPlots()
{
  this->Sigma = 1.;
  this->TransposeTable = false;
  this->Internal = new PVExtractBoxPlotsInternal();
  this->SetNumberOfOutputPorts(2);
}

//----------------------------------------------------------------------------
vtkPVExtractBoxPlots::~vtkPVExtractBoxPlots()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkPVExtractBoxPlots::EnableAttributeArray(const char* arrName)
{
  if (arrName)
    {
    if (this->Internal->Set(arrName))
      {
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkPVExtractBoxPlots::ClearAttributeArrays()
{
  if (this->Internal->Clear())
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkPVExtractBoxPlots::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkPVExtractBoxPlots::RequestData(vtkInformation*,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector* outputVector)
{
  vtkTable* inTable = vtkTable::GetData(inputVector[0]);
  vtkTable* outTable = vtkTable::GetData(outputVector, 0);
  vtkTable* outTable2 = vtkTable::GetData(outputVector, 1);
  
  if (inTable->GetNumberOfColumns() == 0)
    {    
    return 1;
    }

  
  vtkNew<vtkTransposeTable> transpose;

  // Construct a table that holds only the selected columns
  vtkNew<vtkTable> subTable;
  std::set<std::string>::iterator iter = this->Internal->Columns.begin();
  for (; iter!=this->Internal->Columns.end(); ++iter)
    {    
    if (vtkAbstractArray* arr = inTable->GetColumnByName(iter->c_str()))
      {
      subTable->AddColumn(arr);
      }
    }
  
  vtkTable *inputTable = subTable.GetPointer();

  outTable->ShallowCopy(subTable.GetPointer());
  
  if (this->TransposeTable)
    {    
    transpose->SetInputData(subTable.GetPointer());
    transpose->SetAddIdColumn(true);
    transpose->SetIdColumnName("ColName");
    transpose->Update();

    inputTable = transpose->GetOutput();
    }

  outTable2->ShallowCopy(inputTable);
  
  // Compute PCA
    
  vtkNew<vtkPSciVizPCAStats> pca;
  pca->SetInputData(inputTable);
  pca->SetAttributeMode(vtkDataObject::ROW);
  for (vtkIdType i = 0; i < inputTable->GetNumberOfColumns(); i++)
    {
    vtkAbstractArray* arr = inputTable->GetColumn(i);
    if (strcmp(arr->GetName(), "ColName"))
      {
      pca->EnableAttributeArray(arr->GetName());
      }
    }
    
  pca->SetBasisScheme(vtkPCAStatistics::FIXED_BASIS_SIZE);
  pca->SetFixedBasisSize(2);
  pca->Update();
  
  vtkTable* outputPCATable = vtkTable::SafeDownCast(
    pca->GetOutputDataObject(vtkStatisticsAlgorithm::OUTPUT_MODEL));

  outTable2->ShallowCopy(outputPCATable);
  
  // Compute HDR
  
  vtkNew<vtkHighestDensityRegionsStatistics> hdr;
  hdr->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, outputPCATable);
  hdr->SetSigma(this->Sigma);

  std::string x, y;
  for (vtkIdType i = 0; i < outputPCATable->GetNumberOfColumns(); i++)
    {
    vtkAbstractArray* arr = outputPCATable->GetColumn(i);
    char* str = arr->GetName();
    if (strstr(str, "PCA"))
      {
      if (strstr(str, "(0)"))
        {
        x = std::string(str);
        arr->SetName("X");
        }
      else 
        {
        y = std::string(str);
        arr->SetName("Y");
        }
      }
    }
  hdr->AddColumnPair("X", "Y");
  hdr->SetLearnOption(true);
  hdr->SetDeriveOption(true);
  hdr->SetAssessOption(false);
  hdr->SetTestOption(false);
  hdr->Update();
  
  vtkMultiBlockDataSet* outputHDR = vtkMultiBlockDataSet::SafeDownCast(
    hdr->GetOutputDataObject(vtkStatisticsAlgorithm::OUTPUT_MODEL));
  vtkTable* outputHDRTable = vtkTable::SafeDownCast(outputHDR->GetBlock(0));

  outTable2->ShallowCopy(outputHDRTable);

  return 1;
}
