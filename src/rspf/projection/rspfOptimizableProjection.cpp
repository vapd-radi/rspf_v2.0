#include <rspf/projection/rspfOptimizableProjection.h>
RTTI_DEF(rspfOptimizableProjection, "rspfOptimizableProjection");
rspfOptimizableProjection::rspfOptimizableProjection()
{}
rspfOptimizableProjection::rspfOptimizableProjection(
   const rspfOptimizableProjection& /* source */ )
{
}
rspfOptimizableProjection::~rspfOptimizableProjection()
{}
rspfOptimizableProjection& rspfOptimizableProjection::operator=(
   const rspfOptimizableProjection& /* source */ )
{
   return *this;
}
bool rspfOptimizableProjection::setupOptimizer(
   const rspfString& /* setup */ )
{
   return false;
}
bool rspfOptimizableProjection::needsInitialState() const
{
   return false;
}
