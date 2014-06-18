/* -*- C++ -*- */

namespace MFM
{
  template <class GC>
  class FunctionCallRegisterElement : public ConfigFunctionCall<GC>
  {
  public:
    FunctionCallRegisterElement() : ConfigFunctionCall<GC>("RegisterElement")
    { }

    virtual bool Parse(ExternalConfig<GC> & ec)
    {
      LineCountingByteSource & in = ec.GetByteSource();
      in.SkipWhitespace();
      UUID uuid;
      if (!uuid.Read(in))
        return in.Msg(Logger::ERROR, "Error reading Element UUID");
      s32 ret = this->SkipToNextArg(in);
      if (ret < 0) return false;
      if (ret == 0)
        return in.Msg(Logger::ERROR, "Expected second argument");
      OString16 buf;
      if (!in.ScanIdentifier(buf))
        return in.Msg(Logger::ERROR, "Expected identifier as second argument");
      if (buf.HasOverflowed())
        return in.Msg(Logger::ERROR, "Identifier too long '%s'", buf.GetZString());
      if (!ec.RegisterElement(uuid, buf))
        return in.Msg(Logger::ERROR, "Couldn't register '%@' under name '%s'", &uuid, buf.GetZString());

      return this->SkipToNextArg(in) == 0;
    }

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply(ExternalConfig<GC> & ec) { /* Work already done */ }

  };

  template <class GC>
  class FunctionCallGA : public ConfigFunctionCall<GC>
  {
    typedef typename GC::CORE_CONFIG CC;

  public:
    FunctionCallGA() : ConfigFunctionCall<GC>("GA")
    { }

    virtual bool Parse(ExternalConfig<GC> & ec)
    {
      LineCountingByteSource & in = ec.GetByteSource();
      in.SkipWhitespace();

      OString16 nick;
      if (!in.ScanIdentifier(nick))
        return in.Msg(Logger::ERROR, "Expected identifier as first argument");
      if (nick.HasOverflowed())
        return in.Msg(Logger::ERROR, "Identifier too long '%s'", nick.GetZString());

      const Element<CC> * pelt = ec.LookupElement(nick);
      if (!pelt)
        return in.Msg(Logger::ERROR, "'%@' isn't a registered element nickname", &nick);

      s32 x, y;
      s32 ret;

      ret = this->SkipToNextArg(in);
      if (ret < 0) return false;
      if (ret == 0)
        return in.Msg(Logger::ERROR, "Expected second argument");

      if (!in.Scan(x)) return in.Msg(Logger::ERROR, "Expected x position");
      ret = this->SkipToNextArg(in);
      if (ret < 0) return false;
      if (ret == 0)
        return in.Msg(Logger::ERROR, "Expected second argument");

      if (!in.Scan(y)) return in.Msg(Logger::ERROR, "Expected y position");

      if (!ec.PlaceAtom(*pelt, x, y))
        return false;

      return this->SkipToNextArg(in) == 0;
    }

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply(ExternalConfig<GC> & ec) { /* Work already done */ }

  };

  template <class GC>
  class FunctionCallSetParameter : public ConfigFunctionCall<GC>
  {
    typedef typename GC::CORE_CONFIG CC;

  public:
    FunctionCallSetParameter() : ConfigFunctionCall<GC>("SetParameter")
    { }

    virtual bool Parse(ExternalConfig<GC> & ec)
    {
      LineCountingByteSource & in = ec.GetByteSource();
      in.SkipWhitespace();

      OString64 parm;
      if (!in.ScanIdentifier(parm))
        return in.Msg(Logger::ERROR, "Expected parameter name as first argument");

      s32 val;
      s32 ret;

      ret = this->SkipToNextArg(in);
      if (ret < 0) return false;
      if (ret == 0)
        return in.Msg(Logger::ERROR, "Expected second argument (parameter value)");

      if (!in.Scan(val)) return in.Msg(Logger::ERROR, "Expected decimal parameter value");

      in.Msg(Logger::ERROR, "Unimplemented: SetParameter(%s,%d)", parm.GetZString(), val);

      return this->SkipToNextArg(in) == 0;
    }

    virtual void Print(ByteSink & in) { FAIL(UNSUPPORTED_OPERATION); }

    virtual void Apply(ExternalConfig<GC> & ec) { /* Work already done */ }

  };


  template <class GC>
  void RegisterExternalConfigFunctions(ExternalConfig<GC> & ec)
  {
    {
      static FunctionCallRegisterElement<GC> elt;
      ec.RegisterFunction(elt);
    }
    {
      static FunctionCallGA<GC> elt;
      ec.RegisterFunction(elt);
    }
    {
      static FunctionCallSetParameter<GC> elt;
      ec.RegisterFunction(elt);
    }
  }

}
