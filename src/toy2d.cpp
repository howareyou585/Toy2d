#include "toy2d/toy2d.hpp"
//#include "toy2d/context.hpp"
namespace toy2d
{
    void Init(const std::vector<const char*>& extensions, Context::GetSurfaceCallBack cb,int w,int h)
    {
        Context::Init(extensions, cb,w,h);
    }
    void Quit()
    {
        Context::Quit();
    }
}