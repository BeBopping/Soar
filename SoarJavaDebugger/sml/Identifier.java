/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version: 1.3.22
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package sml;

public class Identifier extends WMElement {
  private long swigCPtr;

  protected Identifier(long cPtr, boolean cMemoryOwn) {
    super(smlJNI.SWIGIdentifierUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(Identifier obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected Identifier() {
    this(0, false);
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      throw new UnsupportedOperationException("C++ destructor does not have public access");
    }
    swigCPtr = 0;
    super.delete();
  }

  public String GetValueType() {
    return smlJNI.Identifier_GetValueType(swigCPtr);
  }

  public String GetValueAsString() {
    return smlJNI.Identifier_GetValueAsString(swigCPtr);
  }

  public boolean IsIdentifier() {
    return smlJNI.Identifier_IsIdentifier(swigCPtr);
  }

  public Identifier ConvertToIdentifier() {
    long cPtr = smlJNI.Identifier_ConvertToIdentifier(swigCPtr);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public Identifier FindIdentifier(String pID, int index) {
    long cPtr = smlJNI.Identifier_FindIdentifier(swigCPtr, pID, index);
    return (cPtr == 0) ? null : new Identifier(cPtr, false);
  }

  public WMElement FindTimeTag(int timeTag) {
    long cPtr = smlJNI.Identifier_FindTimeTag(swigCPtr, timeTag);
    return (cPtr == 0) ? null : new WMElement(cPtr, false);
  }

  public WMElement FindByAttribute(String pAttribute, int index) {
    long cPtr = smlJNI.Identifier_FindByAttribute(swigCPtr, pAttribute, index);
    return (cPtr == 0) ? null : new WMElement(cPtr, false);
  }

  public String GetParameterValue(String pAttribute) {
    return smlJNI.Identifier_GetParameterValue(swigCPtr, pAttribute);
  }

  public String GetCommandName() {
    return smlJNI.Identifier_GetCommandName(swigCPtr);
  }

  public void AddStatusComplete() {
    smlJNI.Identifier_AddStatusComplete(swigCPtr);
  }

  public void AddStatusError() {
    smlJNI.Identifier_AddStatusError(swigCPtr);
  }

  public void AddErrorCode(int errorCode) {
    smlJNI.Identifier_AddErrorCode(swigCPtr, errorCode);
  }

  public int GetNumberChildren() {
    return smlJNI.Identifier_GetNumberChildren(swigCPtr);
  }

  public WMElement GetChild(int index) {
    long cPtr = smlJNI.Identifier_GetChild(swigCPtr, index);
    return (cPtr == 0) ? null : new WMElement(cPtr, false);
  }

  public boolean AreChildrenModified() {
    return smlJNI.Identifier_AreChildrenModified(swigCPtr);
  }

}
