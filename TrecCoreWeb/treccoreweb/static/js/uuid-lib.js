var validator = new RegExp("^[a-z0-9]{32}$", "i");

function gen(count) {
  var out = "";
  for (var i=0; i<count; i++) {
    out += (((1+Math.random())*0x10000)|0).toString(16).substring(1);
  }
  return out;
}

function Uuid(uuid) {
  if (!uuid) throw new TypeError("Invalid argument; `value` has no value.");

  var value = Uuid.EMPTY;

  if (uuid && uuid instanceof Uuid) {
    value = Uuid.toString();

  } else if (uuid && Object.prototype.toString.call(uuid) === "[object String]" && Uuid.isUuid(uuid)) {
    value = uuid;
  }

  this.equals = function(other) {
    // Comparing string `value` against provided `uuid` will auto-call
    // toString on `uuid` for comparison
    return Uuid.isUuid(other) && value == other;
  };

  this.isEmpty = function() {
    return value === Uuid.EMPTY;
  };

  this.toString = function() {
    return value;
  };

  this.toJSON = function() {
    return value;
  };

  Object.defineProperty(this, "value", {
    get: function() { return value; },
    enumerable: true
  });
};

Object.defineProperty(Uuid, "EMPTY", {
  value: "00000000000000000000000000000000"
});

Uuid.isUuid = function(value) {
  return value && (value instanceof Uuid || validator.test(value.toString()));
};

Uuid.create = function() {
  return new Uuid(gen(8));
};

Uuid.raw = function() {
  return gen(8);
};

