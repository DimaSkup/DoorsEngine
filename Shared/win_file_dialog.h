#pragma once
#include <windows.h>
#define STRICT_TYPED_ITEMIDS
#include <shlobj.h>
#include <shlwapi.h>
#include "ShellHelpers.h"
#include <strsafe.h>
#include "StrHelper.h"


/*
    Usage:

    CItemIterator itemIterator(psi);

    while (itemIterator.MoveNext())
    {
        IShellItem2 *psi;
        hr = itemIterator.GetCurrent(IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            // Perform action on psi
            psi->Release();
        }
    }
*/
class CItemIterator
{
public:
    CItemIterator(IShellItem* psi) : _hr(SHGetIDListFromObject(psi, &_pidlFull)), _psfCur(NULL)
    {
        _Init();
    }

    CItemIterator(PCIDLIST_ABSOLUTE pidl) : _hr(SHILCloneFull(pidl, &_pidlFull)), _psfCur(NULL)
    {
        _Init();
    }

    ~CItemIterator()
    {
        CoTaskMemFree(_pidlFull);
        SafeRelease(&_psfCur);
    }

    bool MoveNext()
    {
        bool fMoreItems = false;
        if (SUCCEEDED(_hr))
        {
            if (NULL == _pidlRel)
            {
                fMoreItems = true;
                _pidlRel = _pidlFull;   // First item - Might be empty if it is the desktop
            }
            else if (!ILIsEmpty(_pidlRel))
            {
                PCUITEMID_CHILD pidlChild = (PCUITEMID_CHILD)_pidlRel;  // Save the current segment for binding
                _pidlRel = ILNext(_pidlRel);

                // If we are not at the end setup for the next iteration
                if (!ILIsEmpty(_pidlRel))
                {
                    const WORD cbSave = _pidlRel->mkid.cb;  // Avoid cloning for the child by truncating temporarily
                    _pidlRel->mkid.cb = 0;                  // Make this a child

                    IShellFolder* psfNew;
                    _hr = _psfCur->BindToObject(pidlChild, NULL, IID_PPV_ARGS(&psfNew));
                    if (SUCCEEDED(_hr))
                    {
                        _psfCur->Release();
                        _psfCur = psfNew;   // Transfer ownership
                        fMoreItems = true;
                    }

                    _pidlRel->mkid.cb = cbSave; // Restore previous ID size
                }
            }
        }
        return fMoreItems;
    }

    HRESULT GetCurrent(REFIID riid, void** ppv)
    {
        *ppv = NULL;
        if (SUCCEEDED(_hr))
        {
            // Create the childID by truncating _pidlRel temporarily
            PUIDLIST_RELATIVE pidlNext = ILNext(_pidlRel);
            const WORD cbSave = pidlNext->mkid.cb;  // Save old cb
            pidlNext->mkid.cb = 0;                  // Make _pidlRel a child

            _hr = SHCreateItemWithParent(NULL, _psfCur, (PCUITEMID_CHILD)_pidlRel, riid, ppv);

            pidlNext->mkid.cb = cbSave;             // Restore old cb
        }
        return _hr;
    }

    HRESULT GetResult() const { return _hr; }
    PCUIDLIST_RELATIVE GetRelativeIDList() const { return _pidlRel; }

private:
    void _Init()
    {
        _pidlRel = NULL;

        if (SUCCEEDED(_hr))
        {
            _hr = SHGetDesktopFolder(&_psfCur);
        }
    }

    HRESULT _hr;
    PIDLIST_ABSOLUTE _pidlFull;
    PUIDLIST_RELATIVE _pidlRel;
    IShellFolder* _psfCur;
};

//---------------------------------------------------------
// Desc:   get full path to selected file (in the dialog window)
//---------------------------------------------------------
static HRESULT GetIDListName(IShellItem* psi, std::wstring& outPath)
{
    HRESULT hr = E_FAIL;
    LPWSTR path = NULL;
    bool startPushingIntoPath = false;
    CItemIterator itemIterator(psi);

    while (itemIterator.MoveNext())
    {
        IShellItem2* psi = NULL;
        hr = itemIterator.GetCurrent(IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            // when we go our from this func the "path" will contain a full path to file
            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &path);
            if (SUCCEEDED(hr))
            {
                wprintf(L"path: %ls\n", path);
            }

#if 0       // when we go our from this func the fileName will contain filename ;)
            hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEEDITING, &fileName);
            if (SUCCEEDED(hr))
            {
                wprintf(L"filename: %ls\n", fileName);
            }
#endif

            psi->Release();
        }
    }

    outPath = path;
    CoTaskMemFree(path);  // release memory

    return hr;
}

//---------------------------------------------------------

static void DialogWndFileOpen(std::string& outPath)
{
    HRESULT hr = OleInitialize(0);
    std::wstring path;

    if (SUCCEEDED(hr))
    {
        // CoCreate the File Open Dialog object
        IFileDialog* pfd = NULL;

        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
        {
            if (SUCCEEDED(pfd->Show(NULL)))
            {
                IShellItem* psi = NULL;

                if (SUCCEEDED(pfd->GetResult(&psi)))
                {
                    if (SUCCEEDED(GetIDListName(psi, path)))
                    {
                        // check if our path contain only valid symbols
                        if (!FileSys::IsPathSupported(path.c_str()))
                        {
                            LogErr(LOG, "you can't select a file using the dialog window");
                            psi->Release();
                            pfd->Release();
                            OleUninitialize();

                            return;
                        }

                        // convert wide string into char string
                        outPath.resize(path.length());
                        StrHelper::ToStr(path.c_str(), (char*)outPath.c_str());

                        printf("path: %s\n", outPath.c_str());
                        MessageBoxA(NULL, outPath.c_str(), "SelectedItem", MB_OK);
                    }
                    psi->Release();
                }
            }
            pfd->Release();
        }

        OleUninitialize();
    }
}
