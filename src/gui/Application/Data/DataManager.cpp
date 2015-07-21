#include "DataManager.h"
#include <QMessageBox>
#include <Cleaver/InverseField.h>
#include <Cleaver/AbstractScalarField.h>

DataManager::DataManager()
{
}



void DataManager::addMesh(cleaver::TetMesh *mesh)
{
    m_meshes.push_back(mesh);

    emit dataChanged();
    emit dataAdded();
    emit meshAdded();
    emit meshListChanged();
}

void DataManager::removeMesh(cleaver::TetMesh *mesh)
{
    std::vector<cleaver::TetMesh*>::iterator iter = m_meshes.begin();
    while(iter != m_meshes.end())
    {
        if(*iter == mesh)
            iter = m_meshes.erase(iter);
    }

    emit dataChanged();
    emit dataRemoved();
    emit meshRemoved();
    emit meshListChanged();
}

void DataManager::addField(cleaver::AbstractScalarField *field)
{
    m_fields.push_back(field);

    emit dataChanged();
    emit dataAdded();
    emit fieldAdded();
    emit fieldListChanged();
}

void DataManager::removeField(cleaver::AbstractScalarField *field, bool ask_delete_volume)
{
    std::vector<cleaver::AbstractScalarField*>::iterator iter;
	bool deleteVolume = false;
	bool addInverse = false;
	if (m_fields.size() == 2 && ask_delete_volume) {
		QMessageBox msgBox;
		msgBox.setText("You are Removing too many fields");
		msgBox.setInformativeText("Do you want remove the volume, or mesh one field?");
		QPushButton *deleteVol = msgBox.addButton(tr("Remove Volume"), QMessageBox::DestructiveRole);
		QPushButton *addInv = msgBox.addButton(tr("Mesh One"), QMessageBox::ActionRole);
		QPushButton *cancel = msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
		msgBox.exec();

		if (msgBox.clickedButton() == reinterpret_cast<QAbstractButton*>(deleteVol)) {
			deleteVolume = true;
		} else if (msgBox.clickedButton() == reinterpret_cast<QAbstractButton*>(addInv)) {
			addInverse = true;
		} else if (msgBox.clickedButton() == reinterpret_cast<QAbstractButton*>(cancel)) {
			return;
		}
	}
	
    for(iter = m_fields.begin(); iter != m_fields.end(); iter++)
    {        
        // remove field if there's a match
        if(*iter == field){
            iter = m_fields.erase(iter);        

            // make sure we're not at the end
            if(iter == m_fields.end())
                break;
        }
    }

    // remove it from any volume's that have it
    std::vector<cleaver::Volume*>::iterator vol_iter;
    cleaver::Volume *temp_volume = NULL;
    for(vol_iter = m_volumes.begin(); vol_iter != m_volumes.end(); vol_iter++)
    {
        cleaver::Volume *volume = *vol_iter;

        // first check materials
        for(int m=0; m < volume->numberOfMaterials(); m++)
        {
            if(volume->getMaterial(m) == field) {
                volume->removeMaterial(field);
				temp_volume = volume;
			}
        }

        // then check sizing field
        if(volume->getSizingField() == field)
            volume->setSizingField(NULL);
    }

    // finally free the memory
	//if (field)
	//	delete field;

	if (deleteVolume && temp_volume)
		removeVolume(temp_volume);
	if (addInverse && temp_volume) {
		cleaver::AbstractScalarField* fld = new cleaver::InverseScalarField(m_fields[0]);
		fld->setName(m_fields[0]->name() + "-inverse");
		temp_volume->addMaterial(fld);
		addField(fld);
	}


    emit dataChanged();
    emit dataRemoved();
    emit fieldRemoved();
    emit fieldListChanged();
    emit volumeListChanged();
}

void DataManager::addVolume(cleaver::Volume *volume)
{	
    m_volumes.push_back(volume);

    emit dataChanged();
    emit dataAdded();
    emit volumeAdded();
    emit volumeListChanged();
}

void DataManager::removeVolume(cleaver::Volume *volume)
{
    std::vector<cleaver::Volume*>::iterator iter;
    for(iter = m_volumes.begin(); iter != m_volumes.end(); iter++)
    {	
		cleaver::Volume* vol = *iter;
		while (vol->numberOfMaterials() > 0) {
			this->removeField(vol->getMaterial(0),false);
		}
		this->removeField(vol->getSizingField(),false);
        // remove the volume if there's a match
        if(*iter == volume){
            iter = m_volumes.erase(iter);

            // make sure we're not at the end
            if(iter == m_volumes.end())
                break;
        }
    }

    // free memory
    delete volume;

    emit dataChanged();
    emit dataRemoved();
    emit volumeRemoved();
    emit volumeListChanged();
}

std::vector<ulong> DataManager::getSelection()
{
    return m_selection;
}

//============================================
// This method will set the given data object
// to be the exclusive selection.
//============================================
void DataManager::setSelection(ulong ptr)
{
    m_selection.clear();
    m_selection.push_back(ptr);

    emit selectionChanged();
}

//============================================
// This method will add the given data object
// to the selection list. Keeping any prior
// items on the list as well.
//============================================
void DataManager::addSelection(ulong ptr)
{
    // make sure we don't add it twice
    std::vector<ulong>::iterator iter = m_selection.begin();
    while(iter != m_selection.end())
    {
        if(*iter == ptr)
            return;

        iter++;
    }

    m_selection.push_back(ptr);
    emit selectionChanged();
}


//===============================================
// This method will altnerate the given data
// object from selected or not selected, and
// remove any other current selections.
//===============================================
void DataManager::toggleSetSelection(ulong ptr)
{
    std::vector<ulong>::iterator iter = m_selection.begin();

    while(iter != m_selection.end())
    {
        if(*iter == ptr)
        {
            m_selection.clear();
            emit selectionChanged();
            return;
        }

        iter++;
    }

    m_selection.clear();
    m_selection.push_back(ptr);

    std::cout << "Selection Toggled" << std::endl;

    emit selectionChanged();
}

//===============================================
// This method will alternate the given data object
// from selected or not selected. It will not alter
// any other selection in place.
//===============================================

void DataManager::toggleAddSelection(ulong ptr)
{
    std::vector<ulong>::iterator iter = m_selection.begin();

    while(iter != m_selection.end())
    {
        // remove it if we find it
        if(*iter == ptr)
        {
            m_selection.erase(iter);
            emit selectionChanged();
            return;
        }

        iter++;
    }

    // otherwise add it
    m_selection.push_back(ptr);

    std::cout << "Selection Toggled" << std::endl;

    emit selectionChanged();
}

void DataManager::clearSelection()
{
    m_selection.clear();
    emit selectionChanged();
}
