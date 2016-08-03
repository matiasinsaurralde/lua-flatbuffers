#ifndef LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_
#define LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_

#include "encoder_context.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/reflection.h>  // for objects()
#include <LuaIntf/LuaIntf.h>  // for createTable()

#include <sstream>  // for ostringstream
#include <string>

#define ERR_RET(ErrorStr) do { \
	SetError(ErrorStr); \
	return; \
} while(0)

static inline bool IsLuaNumber(const LuaIntf::LuaRef& luaValue)
{
	return luaValue.type() == LuaIntf::LuaTypeID::NUMBER;
}

class EncoderBase
{
public:
	explicit EncoderBase(EncoderContext& rCtx) : m_rCtx(rCtx) {};
	virtual ~EncoderBase() {};

public:
	using LuaRef = LuaIntf::LuaRef;
	using string = std::string;

protected:
	inline void CheckScalarLuaValue(const LuaRef& luaValue);

	template <typename T>
	T LuaToNumber(const LuaRef& luaValue);

protected:
	const flatbuffers::Vector<flatbuffers::Offset<reflection::Object>>&
	Objects() const
	{
		return *m_rCtx.schema.objects();
	}
	flatbuffers::FlatBufferBuilder& Builder() { return m_rCtx.fbb; }
	const flatbuffers::FlatBufferBuilder& Builder() const { return m_rCtx.fbb; }

protected:
	bool Bad() const { return !m_rCtx.sError.empty(); }
	void SetError(const string& sError);
	void PushName(const string& sName) { m_rCtx.nameStack.Push(sName); }
	void PushName(const reflection::Object& object)
	{
		PushName(object.name()->c_str());
	}
	void PushName(const reflection::Field& field)
	{
		PushName(field.name()->c_str());
	}
	void SafePopName() { m_rCtx.nameStack.SafePop(); }
	string PopFullName();
	string PopFullFieldName(const string& sFieldName);
	string PopFullFieldName(const reflection::Field& field);
	string PopFullVectorName(size_t index);

private:
	void SetLuaToIntError(const LuaRef& luaValue);

protected:
	EncoderContext& m_rCtx;
};  // class EncoderBase

void EncoderBase::CheckScalarLuaValue(const LuaRef& luaValue)
{
	if (IsLuaNumber(luaValue)) return;
	SetError("scalar field " + PopFullName() + " is " + luaValue.typeName());
}

template <typename T>
T EncoderBase::LuaToNumber(const LuaRef& luaValue)
{
	static_assert(std::is_scalar<T>::value,
		"LuaToNumber() is only for scalar types.");
	// Directly toValue<int>() may throw.
	LuaRef toInt(luaValue.state(), "math.tointeger");
	LuaRef luaInt = toInt.call<LuaRef>(luaValue);
	if (luaInt) return luaInt.toValue<T>();
	SetLuaToIntError(luaValue);
	return 0;
}

template <> float EncoderBase::LuaToNumber<float>(const LuaRef& luaValue);
template <> double EncoderBase::LuaToNumber<double>(const LuaRef& luaValue);

#endif  // LUA_FLATBUFFERS_ENCODER_ENCODER_BASE_H_
