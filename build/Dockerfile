FROM wichtounet/budgetwarrior:build

EXPOSE 8080

CMD ["/bin/server", ""]

ADD budgetwarrior/tools/yfinance_quote.py /bin/yfinance_quote.py
ADD release_debug/gcc-15/bin/server /bin/server
