import threading

def is_main_thread() -> bool:
    return threading.current_thread() is threading.main_thread()

def assert_main_thread():
    if not is_main_thread():
        raise RuntimeError("This function can only be called from the main thread.")