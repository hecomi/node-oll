#ifndef INCLUDE_V8_HPP
#define INCLUDE_V8_HPP

#include <v8.h>
#include <string>
#include <fstream>
#include <boost/format.hpp>
#include <boost/any.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace hecomi {
	namespace V8 {

// プロトタイプ宣言
template <class T> class ExportToJS;

/* ------------------------------------------------------------------------- */
//  class JavaScript
/* ------------------------------------------------------------------------- */
/**
 * v8 で JavaScript を実行するクラス
 */
class JavaScript
{
public:
	/**
	 * コンストラクタ
	 */
	JavaScript() : global_(v8::ObjectTemplate::New())
	{
		context_ = v8::Context::New(nullptr, global_);
		context_scope_ = boost::make_shared<v8::Context::Scope>(context_);
	}

	/**
	 * デストラクタ
	 * v8::Persistent を片付ける
	 */
	~JavaScript()
	{
		global_.Dispose();
		context_.Dispose();
	}

	/**
	 * JavaScript側へ C++ の関数をエクスポートする
	 * @param[in] func_name 関数名
	 * @param[in] func エクスポートする関数
	 */
	template <class T>
	void set(const std::string& func_name, T func)
	{
		v8::Context::Scope context_scope(v8::Context::GetCurrent());

		v8::Context::GetCurrent()->Global()->Set(
			v8::String::New(func_name.c_str()),
			v8::FunctionTemplate::New(func)->GetFunction()
		);
	}

	/**
	 * JavaScript側へ C++ の関数をエクスポートする
	 * @param[in] clazz エクスポートするクラス
	 */
	template <class T>
	void set(ExportToJS<T> clazz)
	{
		v8::Context::Scope context_scope(v8::Context::GetCurrent());

		v8::Context::GetCurrent()->Global()->Set(
			v8::String::New(clazz.get_class_name().c_str()),
			clazz.get_class()->GetFunction()
		);
	}

	/**
	 *  JavaScript を文字列から実行する
	 *  @param[in] code JavaScript のソースコード
	 *  @return true: 実行成功, false: 実行失敗
	 */
	bool exec(const std::string& code)
	{
		v8::TryCatch try_catch;
		v8::Context::Scope context_scope(v8::Context::GetCurrent());

		// compile
		v8::Handle<v8::String> script = v8::String::New(code.c_str());
		v8::Handle<v8::Script> compiled_script = v8::Script::Compile(script);
		if (compiled_script.IsEmpty()) {
			v8::String::Utf8Value error(try_catch.Exception());
			std::cerr << *error << std::endl;
			return false;
		}

		// run
		v8::Handle<v8::Value> result = compiled_script->Run();
		if (result.IsEmpty()) {
			v8::String::Utf8Value error(try_catch.Exception());
			std::cerr << *error << std::endl;
			return false;
		}

		return true;
	}

	/**
	 *  JavaScript をファイルから実行する
	 *  @param[in] file_name ファイル名
	 *  @return true: 実行成功, false: 実行失敗
	 */
	bool exec_script(const std::string& file_name)
	{
		v8::TryCatch try_catch;

		// read js file
		std::ifstream ifs(file_name.c_str());
		if (ifs.fail()) {
			std::cerr << boost::format("js file open error: %1%\n") % file_name;
			return false;
		}
		std::string buf, code = "";
		while (std::getline(ifs, buf)) {
			code += buf.c_str();
		}

		return exec(code);
	}

	/**
	 *  JavaScript 側の関数を実行する
	 *  @param[in] func_name 関数の名前
	 *  @param[in] argc 引数の数
	 *  @param[in] argv 引数
	 *  @return 関数の実行結果
	 */
	v8::Handle<v8::Value> call(
		const std::string& func_name,
		int argc = 0,
		v8::Handle<v8::Value> argv[] = nullptr)
	{
		v8::Context::Scope context_scope(v8::Context::GetCurrent());

		v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(
			v8::Context::GetCurrent()->Global()->Get( v8::String::New( func_name.c_str() ) )
		);
		return func->Call(v8::Context::GetCurrent()->Global(), argc, argv);
	}

private:
	//! Handle scope
	v8::HandleScope handle_scope_;

	//! Global object
	v8::Persistent<v8::ObjectTemplate> global_;

	//! Context
	v8::Persistent<v8::Context> context_;

	//! Context Scope
	boost::shared_ptr<v8::Context::Scope> context_scope_;
};

/* ------------------------------------------------------------------------- */
//  class ExportToJS
/* ------------------------------------------------------------------------- */
/**
 * v8 へクラスを簡単にエクスポートするためのクラス
 */
template <class ExportClass>
class ExportToJS
{
public:
	/**
	 *  コンストラクタ
	 *  @param[in] class_name 登録するクラス名
	 */
	explicit ExportToJS(const std::string& class_name)
	: class_name_(class_name)
	{
		class_ = v8::FunctionTemplate::New(ExportToJS::construct);
		class_->SetClassName(v8::String::New(class_name.c_str()));
		instance_tmpl_ = class_->InstanceTemplate();
		instance_tmpl_->SetInternalFieldCount(1);
		prototype_tmpl_ = class_->PrototypeTemplate();
	}

	/**
	 *  デストラクタ
	 */
	~ExportToJS()
	{
	}

	/**
	 *  生成した FunctionTemplate を取得
	 *  （class JavaScript で利用）
	 *  @return 色々セットした FunctionTemplate
	 */
	v8::Local<v8::FunctionTemplate> get_class() const
	{
		return class_;
	}

	/**
	 *  登録したクラス名を取得
	 *  @return クラス名
	 */
	std::string get_class_name() const
	{
		return class_name_;
	}

	/**
	 *  プロパティを追加
	 *  @param[in] property_name 追加するプロパティ名
	 */
	template <class T = std::string>
	void add_var(const std::string& property_name)
	{
		instance_tmpl_->SetAccessor(
			v8::String::New(property_name.c_str()),
			ExportToJS::get<T>,
			ExportToJS::set<T>
		);
	}

	/**
	 *  メソッドを追加
	 *  @param[in] property_name 追加するメソッド名
	 */
	template <class T = void>
	void add_func(const std::string& property_name)
	{
		prototype_tmpl_->Set(
			v8::String::New(property_name.c_str()),
			v8::FunctionTemplate::New(ExportToJS::func<T>)
		);
	}

private:
	/**
	 *  JavaScript 側でインスタンス生成時に呼ばれる関数
	 *  @param[in] args JavaScript 側で与えられた引数
	 *  @return JavaScript側へ引き渡すオブジェクト
	 */
	static v8::Handle<v8::Value> construct(const v8::Arguments& args)
	{
		ExportClass* instance = new ExportClass();
		v8::Local<v8::Object> this_obj = args.This();
		this_obj->SetInternalField(0, v8::External::New(instance));
		v8::Persistent<v8::Object> holder = v8::Persistent<v8::Object>::New(this_obj);
		holder.MakeWeak(instance, ExportToJS::destruct);

		// constructor の実行
		instance->func("constructor", args);
		return this_obj;
	}

	/**
	 *  JavaScript 側で GC がはたらいた時に呼ばれる関数
	 *  @param[in] handle オブジェクトハンドル
	 *  @param[in] parameter MakeWeak の第1引数
	 */
	static void destruct(v8::Persistent<v8::Value> handle, void* parameter)
	{
		ExportClass* instance = static_cast<ExportClass*>(parameter);
		delete instance;
		handle.Dispose();
	}

	/**
	 *  JavaScript 側で変数の値を読み出すときに呼ばれる関数
	 *
	 *  C++ 側にプロパティ名から値を問い合わせて v8::Value にして返す
	 *  @param[in] property_name プロパティ名
	 *  @param[in] info オブジェクトとか入ってるよ
	 *  @return
	 */
	template <class T>
	static v8::Handle<v8::Value> get(v8::Local<v8::String> property_name, const v8::AccessorInfo& info)
	{
		ExportClass* instance = static_cast<ExportClass*>(
			v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value()
		);
		v8::String::Utf8Value utf8_property_name(property_name);

		// 型で戻り値を変える
		if (boost::is_same<T, int>::value) {
			return v8::Integer::New(boost::any_cast<int>(instance->get(*utf8_property_name)));
		}
		else if (boost::is_same<T, double>::value) {
			return v8::Number::New(boost::any_cast<double>(instance->get(*utf8_property_name)));
		}
		else if (boost::is_same<T, std::string>::value) {
			return v8::String::New(boost::any_cast<std::string>(instance->get(*utf8_property_name)).c_str());
		}
		else {
			return instance->custom_get(*utf8_property_name);
		}
	}

	/**
	 *  JavaScript 側で変数に値をセットするときに呼ばれる関数
	 *
	 *  C++ 側へプロパティ名から値をセットする
	 *  @param[in] property_name プロパティ名
	 *  @param[in] value セットする値
	 */
	template <class T>
	static void set(v8::Local<v8::String> property_name, v8::Local<v8::Value> value, const v8::AccessorInfo& info)
	{
		ExportClass* instance = static_cast<ExportClass*>(
			v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value()
		);
		v8::String::Utf8Value utf8_class_name(info.Holder()->ToString());
		v8::String::Utf8Value utf8_property_name(property_name);
		if (value->IsInt32()) {
			if (!boost::is_same<T, int>::value) {
				std::cerr << boost::format("Error! Integer was subsituted into %1%.%2%, but it was wrong type.\n")
					% *utf8_class_name
					% *utf8_property_name;
				return;
			}
			instance->set(*utf8_property_name, value->Int32Value());
		}
		else if (value->IsNumber()) {
			if (!boost::is_same<T, double>::value) {
				std::cerr << boost::format("Error! Number was subsituted into %1%.%2%, but it was wrong type.\n")
					% *utf8_class_name
					% *utf8_property_name;
				return;
			}
			instance->set(*utf8_property_name, value->NumberValue());
		}
		else if (value->IsString()) {
			if (!boost::is_same<T, std::string>::value) {
				std::cerr << boost::format("Error! String was subsituted into %1%.%2%, but it was wrong type.\n")
					% *utf8_class_name
					% *utf8_property_name;
				return;
			}
			v8::String::Utf8Value utf8_str(value);
			instance->set(*utf8_property_name, std::string(*utf8_str));
		}
		else {
			instance->custom_set(*utf8_property_name, value);
		}
	}

	/**
	 *  メンバ関数を追加する
	 *  @param[in] args JavaScript側から渡される引数
	 *  @return 関数を実行した戻り値
	 */
	template <class T>
	static v8::Handle<v8::Value> func(const v8::Arguments& args)
	{
		ExportClass* instance = static_cast<ExportClass*>(
			v8::Local<v8::External>::Cast(args.This()->GetInternalField(0))->Value()
		);
		v8::String::Utf8Value utf8_func_name(args.Callee()->GetName());

		if (boost::is_same<T, void>::value) {
			instance->func(*utf8_func_name, args);
			return v8::Undefined();
		}
		else if (boost::is_same<T, int>::value) {
			int result = boost::any_cast<int>(instance->func(*utf8_func_name, args));
			return v8::Integer::New(result);
		}
		else if (boost::is_same<T, double>::value) {
			double result = boost::any_cast<double>(instance->func(*utf8_func_name, args));
			return v8::Number::New(result);
		}
		else if (boost::is_same<T, std::string>::value) {
			std::string result = boost::any_cast<std::string>(instance->func(*utf8_func_name, args));
			return v8::String::New(result.c_str());
		}
		else if (boost::is_same<T, bool>::value) {
			bool result = boost::any_cast<bool>(instance->func(*utf8_func_name, args));
			return v8::Boolean::New(result);
		}
		else {
			v8::Handle<v8::Value> result = instance->custom_func(*utf8_func_name, args);
			return result;
		}
	}

	const std::string class_name_;
	v8::Local<v8::FunctionTemplate> class_;
	v8::Local<v8::ObjectTemplate> instance_tmpl_;
	v8::Local<v8::ObjectTemplate> prototype_tmpl_;
};

/* ------------------------------------------------------------------------- */
//  class ExportToJSIF
/* ------------------------------------------------------------------------- */
/**
 * v8 へエクスポートするクラスのひな形
 */
class ExportToJSIF
{
public:
	//! メンバ変数の値を取得 (int, double, std::string 版）
	boost::any get(const std::string& property_name)
	{
		std::cerr << "Error! ExportTOJSIF::get is not overrided." << std::endl;
		return 0;
	}

	//! メンバ変数の値を取得 （その他の型版）
	v8::Handle<v8::Value> custom_get(const std::string& property_name)
	{
		std::cerr << "Error! ExportTOJSIF::custom_get is not overrided." << std::endl;
		return v8::Undefined();
	}

	//! メンバ変数に値をセット (int, double, std::string 版）
	template <class T>
	void set(const std::string& property_name, T value)
	{
		std::cerr << "Error! ExportTOJSIF::set is not overrided." << std::endl;
	}

	//! メンバ変数に値をセット (その他の型版）
	void custom_set(const std::string& property_name, v8::Handle<v8::Value> value)
	{
		std::cerr << "Error! ExportTOJSIF::custom_set is not overrided." << std::endl;
	}

	//! 関数を実行 (int, double, std::string 版）
	boost::any func(const std::string& func_name, const v8::Arguments& args)
	{
		std::cerr << "Error! ExportTOJSIF::func is not overrided." << std::endl;
		return 0;
	}

	//! 関数を実行 (int, double, std::string 版）
	v8::Handle<v8::Value> custom_func(const std::string& func_name, const v8::Arguments& args)
	{
		std::cerr << "Error! ExportTOJSIF::custom_func is not overrided." << std::endl;
		return v8::Undefined();
	}
};

	} // namespace V8
} // namespace hecomi

#endif // INCLUDE_V8_HPP
