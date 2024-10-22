#include <gum/devkit/frida-gum.h>

#include <string>
#include <iostream>

struct _HookListener
{
    GObject parent;

    guint num_calls;

    static void InterfaceInit(gpointer g_iface, gpointer iface_data);
};

G_DECLARE_FINAL_TYPE(HookListener, HookListener, Hook, Listener, GObject);
G_DEFINE_TYPE_EXTENDED(
    HookListener,
    HookListener,
    G_TYPE_OBJECT,
    0,
    G_IMPLEMENT_INTERFACE(
        GUM_TYPE_INVOCATION_LISTENER,
        HookListener::InterfaceInit));

void HookListener_class_init(HookListenerClass *klass)
{
}

void HookListener_init(HookListener *self)
{
}

bool TestHook(const std::string &text)
{
    std::cout << "TestHook called with: \"" << text << "\"" << std::endl
              << std::endl;

    return text == "Hello, world!";
}

int main()
{
    gum_init_embedded();

    auto interceptor = gum_interceptor_obtain();
    auto listener = GUM_INVOCATION_LISTENER(g_object_new(HookListener_get_type(), nullptr));

    gum_interceptor_begin_transaction(interceptor);
    gum_interceptor_attach(interceptor, reinterpret_cast<gpointer>(&TestHook), listener, const_cast<char *>("TestHook"));
    gum_interceptor_end_transaction(interceptor);

    auto testHookResult = TestHook("Hello, world!");
    std::cout << "TestHook result: " << std::boolalpha << testHookResult << std::endl;

    gum_interceptor_detach(interceptor, listener);

    g_object_unref(listener);
    g_object_unref(interceptor);

    gum_deinit_embedded();

    return 0;
}

void HookListener::InterfaceInit(gpointer g_iface, gpointer iface_data)
{
    auto listener = reinterpret_cast<GumInvocationListenerInterface *>(g_iface);

    listener->on_enter = [](GumInvocationListener *listener, GumInvocationContext *context)
    {
        auto self = Hook_Listener(listener);
        auto name = reinterpret_cast<char *>(gum_invocation_context_get_listener_function_data(context));
        auto arg00 = reinterpret_cast<std::string *>(gum_invocation_context_get_nth_argument(context, 0));

        self->num_calls++;
        std::cout << "HookListener::on_enter " << name << std::endl;
        std::cout << "  listener num_calls: " << self->num_calls << std::endl;
        std::cout << "  arg00: \"" << *arg00 << "\"" << std::endl;
        std::cout << std::endl;

        arg00->append(" from hook");
    };
    listener->on_leave = [](GumInvocationListener *listener, GumInvocationContext *context)
    {
        auto name = reinterpret_cast<char *>(gum_invocation_context_get_listener_function_data(context));
        auto returnValue = static_cast<bool>(GPOINTER_TO_UINT(gum_invocation_context_get_return_value(context)));

        std::cout << "HookListener::on_leave " << name << std::endl;
        std::cout << "  returnValue: " << std::boolalpha << returnValue << std::endl;
        std::cout << std::endl;

        gum_invocation_context_replace_return_value(context, GUINT_TO_POINTER(true));
    };
}