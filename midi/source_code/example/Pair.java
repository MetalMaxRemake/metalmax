package org.example;


import java.io.Serializable;

public final class Pair<A, B> implements Serializable, Comparable<Pair<A, B>> {
    private final A first;
    private final B second;

    public Pair(A var1, B var2) {
        this.first = var1;
        this.second = var2;
    }

    public final A component1() {
        return this.first;
    }

    public final B component2() {
        return this.second;
    }

    public boolean equals(Object var1) {
        if (this != var1) {
            if (!(var1 instanceof Pair)) {
                return false;
            }

            Pair var2 = (Pair) var1;
            if (!(this.first == var2.first) || !(this.second == var2.second)) {
                return false;
            }
        }

        return true;
    }

    public final A getFirst() {
        return this.first;
    }

    public final B getSecond() {
        return this.second;
    }

    public int hashCode() {
        Object var3 = this.first;
        int var2 = 0;
        int var1;
        if (var3 != null) {
            var1 = var3.hashCode();
        } else {
            var1 = 0;
        }

        var3 = this.second;
        if (var3 != null) {
            var2 = var3.hashCode();
        }

        return var1 * 31 + var2;
    }

    public String toString() {
        StringBuilder var1 = new StringBuilder();
        var1.append('(');
        var1.append(this.first);
        var1.append(", ");
        var1.append(this.second);
        var1.append(')');
        return var1.toString();
    }

    @Override
    public int compareTo(Pair<A, B> o) {
        if (o.getFirst() instanceof Integer) {
            System.out.println("sort");
            return Integer.compare((Integer) this.getFirst(), (Integer) o.getFirst());
        }
        return 0;
    }
}
