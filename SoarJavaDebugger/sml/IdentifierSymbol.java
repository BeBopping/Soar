/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class IdentifierSymbol {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected IdentifierSymbol(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(IdentifierSymbol obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected IdentifierSymbol() {
    this(0, false);
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      smlJNI.delete_IdentifierSymbol(swigCPtr);
    }
    swigCPtr = 0;
  }

  public IdentifierSymbol(Identifier pIdentifier) {
    this(smlJNI.new_IdentifierSymbol(Identifier.getCPtr(pIdentifier)), true);
  }

  public String GetIdentifierSymbol() {
    return smlJNI.IdentifierSymbol_GetIdentifierSymbol(swigCPtr);
  }

  public void SetIdentifierSymbol(String pID) {
    smlJNI.IdentifierSymbol_SetIdentifierSymbol(swigCPtr, pID);
  }

  public boolean AreChildrenModified() {
    return smlJNI.IdentifierSymbol_AreChildrenModified(swigCPtr);
  }

  public void SetAreChildrenModified(boolean state) {
    smlJNI.IdentifierSymbol_SetAreChildrenModified(swigCPtr, state);
  }

  public void NoLongerUsedBy(Identifier pIdentifier) {
    smlJNI.IdentifierSymbol_NoLongerUsedBy(swigCPtr, Identifier.getCPtr(pIdentifier));
  }

  public int GetNumberUsing() {
    return smlJNI.IdentifierSymbol_GetNumberUsing(swigCPtr);
  }

  public void AddChild(WMElement pWME) {
    smlJNI.IdentifierSymbol_AddChild(swigCPtr, WMElement.getCPtr(pWME));
  }

  public void RemoveChild(WMElement pWME) {
    smlJNI.IdentifierSymbol_RemoveChild(swigCPtr, WMElement.getCPtr(pWME));
  }

}
