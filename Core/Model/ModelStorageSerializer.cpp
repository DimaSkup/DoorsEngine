#include "ModelStorageSerializer.h"
#include <cassert>



ModelStorageSerializer::ModelStorageSerializer()
{
}

///////////////////////////////////////////////////////////

void ModelStorageSerializer::WriteHeader(
    std::ofstream& fout,
    const size numModels,
    const ModelID lastModelID)
{

    fout << "*****************__Model_Storage_Data__*****************\n";
    fout << "#LastID: " << lastModelID << '\n';
    fout << "#ModelsCount: " << numModels << "\n\n";

    fout << "******__ModelsList(model_id,imported_model_path)__******\n";
}

///////////////////////////////////////////////////////////

void ModelStorageSerializer::WriteModelsInfo(
    std::ofstream& fout,
    const ModelID* ids,
    const std::string* paths,
    const size numModels)
{
    assert((ids != nullptr) && (paths != nullptr) && (numModels > 0) && "invalid input args");

    // write info about this model into the data file
    for (index i = 1; i < numModels; ++i)
    {
        fout << ids[i] << ' ' << paths[i] << '\n';
    }
}
